/**
 * Describes functions for converting ARM CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "b64.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-arm.h"

//Private pre-definitions.
json_object* cper_arm_error_info_to_ir(EFI_ARM_ERROR_INFORMATION_ENTRY* error_info);
json_object* cper_arm_processor_context_to_ir(EFI_ARM_CONTEXT_INFORMATION_HEADER* header, void** cur_pos);
json_object* cper_arm_cache_tlb_error_to_ir(EFI_ARM_CACHE_ERROR_STRUCTURE* cache_tlb_error, EFI_ARM_ERROR_INFORMATION_ENTRY* error_info);
json_object* cper_arm_bus_error_to_ir(EFI_ARM_BUS_ERROR_STRUCTURE* bus_error);
json_object* cper_arm_misc_register_array_to_ir(EFI_ARM_MISC_CONTEXT_REGISTER* misc_register);

//Converts the given processor-generic CPER section into JSON IR.
json_object* cper_section_arm_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_ARM_ERROR_RECORD* record = (EFI_ARM_ERROR_RECORD*)section;
    json_object* section_ir = json_object_new_object();
    
    //Validation bits.
    json_object* validation = bitfield_to_ir(record->ValidFields, 4, ARM_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Number of error info and context info structures, and length.
    json_object_object_add(section_ir, "errorInfoNum", json_object_new_int(record->ErrInfoNum));
    json_object_object_add(section_ir, "contextInfoNum", json_object_new_int(record->ContextInfoNum));
    json_object_object_add(section_ir, "sectionLength", json_object_new_uint64(record->SectionLength));

    //Error affinity.
    json_object* error_affinity = json_object_new_object();
    json_object_object_add(error_affinity, "value", json_object_new_int(record->ErrorAffinityLevel));
    json_object_object_add(error_affinity, "type", 
        json_object_new_string(record->ErrorAffinityLevel < 4 ? "Vendor Defined" : "Reserved"));
    json_object_object_add(section_ir, "errorAffinity", error_affinity);

    //Processor ID (MPIDR_EL1) and chip ID (MIDR_EL1).
    json_object_object_add(section_ir, "mpidrEl1", json_object_new_uint64(record->MPIDR_EL1));
    json_object_object_add(section_ir, "midrEl1", json_object_new_uint64(record->MIDR_EL1));

    //Whether the processor is running, and the state of it if so.
    json_object_object_add(section_ir, "running", json_object_new_boolean(record->RunningState & 0b1));
    if (record->RunningState >> 31)
    {
        //Bit 32 of running state is on, so PSCI state information is included.
        //This can't be made human readable, as it is unknown whether this will be the pre-PSCI 1.0 format
        //or the newer Extended StateID format.
        json_object_object_add(section_ir, "psciState", json_object_new_uint64(record->PsciState));
    }

    //Processor error structures.
    json_object* error_info_array = json_object_new_array();
    EFI_ARM_ERROR_INFORMATION_ENTRY* cur_error = (EFI_ARM_ERROR_INFORMATION_ENTRY*)(record + 1);
    for (int i=0; i<record->ErrInfoNum; i++) 
    {
        json_object_array_add(error_info_array, cper_arm_error_info_to_ir(cur_error));
        cur_error++;
    }
    json_object_object_add(section_ir, "errorInfo", error_info_array);

    //Processor context structures.
    //The current position is moved within the processing, as it is a dynamic size structure.
    void* cur_pos = (void*)cur_error;
    json_object* context_info_array = json_object_new_array();
    for (int i=0; i<record->ContextInfoNum; i++)
    {
        EFI_ARM_CONTEXT_INFORMATION_HEADER* header = (EFI_ARM_CONTEXT_INFORMATION_HEADER*)cur_pos;
        json_object* processor_context = cper_arm_processor_context_to_ir(header, &cur_pos);
        json_object_array_add(context_info_array, processor_context);
    }
    json_object_object_add(section_ir, "contextInfo", context_info_array);

    //Is there any vendor-specific information following?
    if (cur_pos < section + record->SectionLength)
    {
        json_object* vendor_specific = json_object_new_object();
        char* encoded = b64_encode((unsigned char*)cur_pos, section + record->SectionLength - cur_pos);
        json_object_object_add(vendor_specific, "data", json_object_new_string(encoded));
        free(encoded);

        json_object_object_add(section_ir, "vendorSpecificInfo", vendor_specific);
    }

    return section_ir;
}

//Converts a single ARM Process Error Information structure into JSON IR.
json_object* cper_arm_error_info_to_ir(EFI_ARM_ERROR_INFORMATION_ENTRY* error_info)
{
    json_object* error_info_ir = json_object_new_object();

    //Version, length.
    json_object_object_add(error_info_ir, "version", json_object_new_int(error_info->Version));
    json_object_object_add(error_info_ir, "length", json_object_new_int(error_info->Length));

    //Validation bitfield.
    json_object* validation = bitfield_to_ir(error_info->ValidationBits, 5, ARM_ERROR_INFO_ENTRY_VALID_BITFIELD_NAMES);
    json_object_object_add(error_info_ir, "validationBits", validation);

    //The type of error information in this log.
    json_object* error_type = integer_to_readable_pair(error_info->Type, 4,
        ARM_ERROR_INFO_ENTRY_INFO_TYPES_KEYS,
        ARM_ERROR_INFO_ENTRY_INFO_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(error_info_ir, "errorType", error_type);

    //Multiple error count.
    json_object* multiple_error = json_object_new_object();
    json_object_object_add(multiple_error, "value", json_object_new_int(error_info->MultipleError));
    json_object_object_add(multiple_error, "type", 
        json_object_new_string(error_info->MultipleError < 1 ? "Single Error" : "Multiple Errors"));
    json_object_object_add(error_info_ir, "multipleError", multiple_error);

    //Flags.
    json_object* flags = bitfield_to_ir(error_info->Flags, 4, ARM_ERROR_INFO_ENTRY_FLAGS_NAMES);
    json_object_object_add(error_info_ir, "flags", flags);

    //Error information, split by type.
    json_object* error_subinfo = NULL;
    switch (error_info->Type)
    {
        case 0: //Cache
        case 1: //TLB
            error_subinfo = cper_arm_cache_tlb_error_to_ir((EFI_ARM_CACHE_ERROR_STRUCTURE*)&error_info->ErrorInformation, error_info);
            break;
        case 2: //Bus
            error_subinfo = cper_arm_bus_error_to_ir((EFI_ARM_BUS_ERROR_STRUCTURE*)&error_info->ErrorInformation);
            break;
    }
    json_object_object_add(error_info_ir, "errorInformation", error_subinfo);

    //Virtual fault address, physical fault address.
    json_object_object_add(error_info_ir, "virtualFaultAddress", json_object_new_uint64(error_info->VirtualFaultAddress));
    json_object_object_add(error_info_ir, "physicalFaultAddress", json_object_new_uint64(error_info->PhysicalFaultAddress));
    
    return error_info_ir;
}

//Converts a single ARM cache/TLB error information structure into JSON IR format.
json_object* cper_arm_cache_tlb_error_to_ir(EFI_ARM_CACHE_ERROR_STRUCTURE* cache_tlb_error, EFI_ARM_ERROR_INFORMATION_ENTRY* error_info)
{
    json_object* cache_tlb_error_ir = json_object_new_object();

    //Validation bitfield.
    json_object* validation = bitfield_to_ir(cache_tlb_error->ValidationBits, 7, ARM_CACHE_TLB_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(cache_tlb_error_ir, "validationBits", validation);

    //Transaction type.
    json_object* transaction_type = integer_to_readable_pair(cache_tlb_error->TransactionType, 3,
        ARM_ERROR_TRANSACTION_TYPES_KEYS,
        ARM_ERROR_TRANSACTION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(cache_tlb_error_ir, "transactionType", transaction_type);

    //Operation.
    json_object* operation;
    if (error_info->Type == 0)
    {
        //Cache operation.
        operation = integer_to_readable_pair(cache_tlb_error->Operation, 11,
            ARM_CACHE_BUS_OPERATION_TYPES_KEYS,
            ARM_CACHE_BUS_OPERATION_TYPES_VALUES,
            "Unknown (Reserved)");
    }
    else
    {
        //TLB operation.
        operation = integer_to_readable_pair(cache_tlb_error->Operation, 9,
            ARM_TLB_OPERATION_TYPES_KEYS,
            ARM_TLB_OPERATION_TYPES_VALUES,
            "Unknown (Reserved)");
    }
    json_object_object_add(cache_tlb_error_ir, "operation", operation);

    //Miscellaneous remaining fields.
    json_object_object_add(cache_tlb_error_ir, "level", json_object_new_int(cache_tlb_error->Level));
    json_object_object_add(cache_tlb_error_ir, "processorContextCorrupt", json_object_new_boolean(cache_tlb_error->ProcessorContextCorrupt));
    json_object_object_add(cache_tlb_error_ir, "corrected", json_object_new_boolean(cache_tlb_error->Corrected));
    json_object_object_add(cache_tlb_error_ir, "precisePC", json_object_new_boolean(cache_tlb_error->PrecisePC));
    json_object_object_add(cache_tlb_error_ir, "restartablePC", json_object_new_boolean(cache_tlb_error->RestartablePC));
    return cache_tlb_error_ir;
}

//Converts a single ARM bus error information structure into JSON IR format.
json_object* cper_arm_bus_error_to_ir(EFI_ARM_BUS_ERROR_STRUCTURE* bus_error)
{
    json_object* bus_error_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(bus_error->ValidationBits, 7, ARM_BUS_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(bus_error_ir, "validationBits", validation);

    //Transaction type.
    json_object* transaction_type = integer_to_readable_pair(bus_error->TransactionType, 3,
        ARM_ERROR_TRANSACTION_TYPES_KEYS,
        ARM_ERROR_TRANSACTION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(bus_error_ir, "transactionType", transaction_type);

    //Operation.
    json_object* operation = integer_to_readable_pair(bus_error->Operation, 7,
        ARM_CACHE_BUS_OPERATION_TYPES_KEYS,
        ARM_CACHE_BUS_OPERATION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(bus_error_ir, "operation", operation);

    //Affinity level of bus error, + miscellaneous fields.
    json_object_object_add(bus_error_ir, "level", json_object_new_int(bus_error->Level));
    json_object_object_add(bus_error_ir, "processorContextCorrupt", json_object_new_boolean(bus_error->ProcessorContextCorrupt));
    json_object_object_add(bus_error_ir, "corrected", json_object_new_boolean(bus_error->Corrected));
    json_object_object_add(bus_error_ir, "precisePC", json_object_new_boolean(bus_error->PrecisePC));
    json_object_object_add(bus_error_ir, "restartablePC", json_object_new_boolean(bus_error->RestartablePC));
    json_object_object_add(bus_error_ir, "timedOut", json_object_new_boolean(bus_error->TimeOut));

    //Participation type.
    json_object* participation_type = integer_to_readable_pair(bus_error->ParticipationType, 4,
        ARM_BUS_PARTICIPATION_TYPES_KEYS,
        ARM_BUS_PARTICIPATION_TYPES_VALUES,
        "Unknown");
    json_object_object_add(bus_error_ir, "participationType", participation_type);

    //Address space.
    json_object* address_space = integer_to_readable_pair(bus_error->AddressSpace, 3,
        ARM_BUS_ADDRESS_SPACE_TYPES_KEYS,
        ARM_BUS_ADDRESS_SPACE_TYPES_VALUES,
        "Unknown");
    json_object_object_add(bus_error_ir, "addressSpace", address_space);

    //Memory access attributes.
    //todo: find the specification of these in the ARM ARM
    json_object_object_add(bus_error_ir, "memoryAttributes", json_object_new_int(bus_error->MemoryAddressAttributes));

    //Access Mode
    json_object* access_mode = json_object_new_object();
    json_object_object_add(access_mode, "value", json_object_new_int(bus_error->AccessMode));
    json_object_object_add(access_mode, "name", json_object_new_string(bus_error->AccessMode == 0 ? "Secure" : "Normal"));
    json_object_object_add(bus_error_ir, "accessMode", access_mode);

    return bus_error_ir;
}

//Converts a single ARM processor context block into JSON IR.
json_object* cper_arm_processor_context_to_ir(EFI_ARM_CONTEXT_INFORMATION_HEADER* header, void** cur_pos)
{
    json_object* context_ir = json_object_new_object();

    //Add the context type.
    json_object* context_type = integer_to_readable_pair(header->RegisterContextType, 9,
        ARM_PROCESSOR_INFO_REGISTER_CONTEXT_TYPES_KEYS,
        ARM_PROCESSOR_INFO_REGISTER_CONTEXT_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(context_ir, "registerContextType", context_type);

    //Register array size (bytes).
    json_object_object_add(context_ir, "registerArraySize", json_object_new_uint64(header->RegisterArraySize));

    //The register array itself.
    *cur_pos = (void*)(header + 1);
    json_object* register_array = NULL;
    switch (header->RegisterContextType)
    {
        case EFI_ARM_CONTEXT_TYPE_AARCH32_GPR:
            register_array = uniform_struct_to_ir((UINT32*)cur_pos, 
                sizeof(EFI_ARM_V8_AARCH32_GPR) / sizeof(UINT32), ARM_AARCH32_GPR_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH32_EL1:
            register_array = uniform_struct_to_ir((UINT32*)cur_pos, 
                sizeof(EFI_ARM_AARCH32_EL1_CONTEXT_REGISTERS) / sizeof(UINT32), ARM_AARCH32_EL1_REGISTER_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH32_EL2:
            register_array = uniform_struct_to_ir((UINT32*)cur_pos, 
                sizeof(EFI_ARM_AARCH32_EL2_CONTEXT_REGISTERS) / sizeof(UINT32), ARM_AARCH32_EL2_REGISTER_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH32_SECURE:
            register_array = uniform_struct_to_ir((UINT32*)cur_pos, 
                sizeof(EFI_ARM_AARCH32_SECURE_CONTEXT_REGISTERS) / sizeof(UINT32), ARM_AARCH32_SECURE_REGISTER_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH64_GPR:
            register_array = uniform_struct64_to_ir((UINT64*)cur_pos, 
                sizeof(EFI_ARM_V8_AARCH64_GPR) / sizeof(UINT64), ARM_AARCH64_GPR_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH64_EL1:
            register_array = uniform_struct64_to_ir((UINT64*)cur_pos, 
                sizeof(EFI_ARM_AARCH64_EL1_CONTEXT_REGISTERS) / sizeof(UINT64), ARM_AARCH64_EL1_REGISTER_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH64_EL2:
            register_array = uniform_struct64_to_ir((UINT64*)cur_pos, 
                sizeof(EFI_ARM_AARCH64_EL2_CONTEXT_REGISTERS) / sizeof(UINT64), ARM_AARCH64_EL2_REGISTER_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_AARCH64_EL3:
            register_array = uniform_struct64_to_ir((UINT64*)cur_pos, 
                sizeof(EFI_ARM_AARCH64_EL3_CONTEXT_REGISTERS) / sizeof(UINT64), ARM_AARCH64_EL3_REGISTER_NAMES);
            break;
        case EFI_ARM_CONTEXT_TYPE_MISC:
            register_array = cper_arm_misc_register_array_to_ir((EFI_ARM_MISC_CONTEXT_REGISTER*)cur_pos);
            break;
        default:
            //Unknown register array type, add as base64 data instead.
            register_array = json_object_new_object();
            char* encoded = b64_encode((unsigned char*)cur_pos, header->RegisterArraySize);
            json_object_object_add(register_array, "data", json_object_new_string(encoded));
            free(encoded);
            break;
    }
    json_object_object_add(context_ir, "registerArray", register_array);

    //Set the current position to after the processor context structure.
    *cur_pos = (UINT8*)(*cur_pos) + header->RegisterArraySize;

    return context_ir;
}

//Converts a single CPER ARM miscellaneous register array to JSON IR format.
json_object* cper_arm_misc_register_array_to_ir(EFI_ARM_MISC_CONTEXT_REGISTER* misc_register)
{
    json_object* register_array = json_object_new_object();
    json_object* mrs_encoding = json_object_new_object();
    json_object_object_add(mrs_encoding, "op2", json_object_new_uint64(misc_register->MrsOp2));
    json_object_object_add(mrs_encoding, "crm", json_object_new_uint64(misc_register->MrsCrm));
    json_object_object_add(mrs_encoding, "crn", json_object_new_uint64(misc_register->MrsCrn));
    json_object_object_add(mrs_encoding, "op1", json_object_new_uint64(misc_register->MrsOp1));
    json_object_object_add(mrs_encoding, "o0", json_object_new_uint64(misc_register->MrsO0));
    json_object_object_add(register_array, "mrsEncoding", mrs_encoding);
    json_object_object_add(register_array, "value", json_object_new_uint64(misc_register->Value));

    return register_array;
}