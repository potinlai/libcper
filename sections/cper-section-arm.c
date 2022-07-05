/**
 * Describes functions for converting ARM CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-arm.h"

//Private pre-definitions.
json_object* cper_arm_error_info_to_ir(EFI_ARM_PROCESSOR_ERROR_INFORMATION_ENTRY* error_info, void** cur_pos);
json_object* cper_arm_cache_error_to_ir(EFI_ARM_PROCESSOR_CACHE_ERROR_STRUCTURE* cache_error);
json_object* cper_arm_tlb_error_to_ir(EFI_ARM_PROCESSOR_TLB_ERROR_STRUCTURE* tlb_error);
json_object* cper_arm_bus_error_to_ir(EFI_ARM_PROCESSOR_BUS_ERROR_STRUCTURE* bus_error);

//Converts the given processor-generic CPER section into JSON IR.
json_object* cper_section_arm_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_ARM_PROCESSOR_ERROR_RECORD* record = (EFI_ARM_PROCESSOR_ERROR_RECORD*)section;
    json_object* section_ir = json_object_new_object();
    
    //Validation bits.
    json_object* validation = bitfield_to_ir(record->ValidFields, 4, ARM_PROCESSOR_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Number of error info and context info structures, and length.
    json_object_object_add(section_ir, "errorInfoNum", json_object_new_int(record->ErrInfoNum));
    json_object_object_add(section_ir, "contextInfoNum", json_object_new_int(record->ContextInfoNum));
    json_object_object_add(section_ir, "sectionLength", json_object_new_int(record->SectionLength));

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
    json_object_object_add(section_ir, "running", json_object_new_boolean(record->RunningState));
    if (record->RunningState >> 31)
    {
        //Bit 32 of running state is on, so PSCI state information is included.
        //todo: Look at how to make this human readable from the ARM PSCI document.
        json_object_object_add(section_ir, "psciState", json_object_new_int(record->PsciState));
    }

    //Processor error structures.
    json_object* error_info_array = json_object_new_array();
    EFI_ARM_PROCESSOR_ERROR_INFORMATION_ENTRY* cur_error = (EFI_ARM_PROCESSOR_ERROR_INFORMATION_ENTRY*)(record + 1);
    for (int i=0; i<record->ErrInfoNum; i++) 
    {
        json_object_array_add(error_info_array, cper_arm_error_info_to_ir(cur_error, (void*)&cur_error));
        //Dynamically sized structure, so pointer is controlled within the above function.
    }
    return section_ir;
}

//Converts a single ARM Process Error Information structure into JSON IR.
json_object* cper_arm_error_info_to_ir(EFI_ARM_PROCESSOR_ERROR_INFORMATION_ENTRY* error_info, void** cur_pos)
{
    json_object* error_info_ir = json_object_new_object();

    //Version, length.
    json_object_object_add(error_info_ir, "version", json_object_new_int(error_info->Version));
    json_object_object_add(error_info_ir, "version", json_object_new_int(error_info->Length));

    //Validation bitfield.
    json_object* validation = bitfield_to_ir(error_info->ValidationBits, 5, ARM_PROCESSOR_ERROR_INFO_ENTRY_VALID_BITFIELD_NAMES);
    json_object_object_add(error_info_ir, "validationBits", validation);

    //The type of error information in this log.
    //todo: The UEFI spec is ambiguous, what are the values for these??
    json_object* error_type = integer_to_readable_pair(error_info->Type, 4,
        ARM_PROCESSOR_ERROR_INFO_ENTRY_INFO_TYPES_KEYS,
        ARM_PROCESSOR_ERROR_INFO_ENTRY_INFO_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(error_info_ir, "errorType", error_type);

    //Multiple error count.
    json_object* multiple_error = json_object_object_create();
    json_object_object_add(multiple_error, "value", json_object_new_int(error_info->MultipleError));
    json_object_object_add(multiple_error, "type", 
        json_object_new_string(error_info->MultipleError < 1 ? "Single Error" : "Multiple Errors"));
    json_object_object_add(error_info_ir, "multipleError", multiple_error);

    //Flags.
    json_object* flags = bitfield_to_ir(error_info->Flags, 4, ARM_PROCESSOR_ERROR_INFO_ENTRY_FLAGS_NAMES);
    json_object_object_add(error_info_ir, "flags", flags);

    //Error information, split by type.
    json_object* error_subinfo = NULL;
    switch (error_info->Type)
    {
        case 0: //Cache
            error_subinfo = cper_arm_cache_error_to_ir((EFI_ARM_PROCESSOR_CACHE_ERROR_STRUCTURE*)error_info->ErrorInformation);
            break;
        case 1: //TLB
            error_subinfo = cper_arm_tlb_error_to_ir((EFI_ARM_PROCESSOR_TLB_ERROR_STRUCTURE*)error_info->ErrorInformation);
            break;
        case 2: //Bus
            error_subinfo = cper_arm_bus_error_to_ir((EFI_ARM_PROCESSOR_BUS_ERROR_STRUCTURE*)error_info->ErrorInformation);
            break;
    }
    json_object_object_add(error_info_ir, "errorInformation", error_subinfo);

    return error_info_ir;
}

//Converts a single ARM cache error information structure into JSON IR format.
json_object* cper_arm_cache_error_to_ir(EFI_ARM_PROCESSOR_CACHE_ERROR_STRUCTURE* cache_error)
{
    //todo
}

//Converts a single ARM TLB error information structure into JSON IR format.
json_object* cper_arm_tlb_error_to_ir(EFI_ARM_PROCESSOR_TLB_ERROR_STRUCTURE* tlb_error)
{
    //todo
}

//Converts a single ARM bus error information structure into JSON IR format.
json_object* cper_arm_bus_error_to_ir(EFI_ARM_PROCESSOR_BUS_ERROR_STRUCTURE* bus_error)
{
    //todo
}