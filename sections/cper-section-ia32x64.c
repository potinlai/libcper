/**
 * Describes functions for converting IA32/x64 CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "b64.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-ia32x64.h"

//Private pre-definitions.
json_object* cper_ia32x64_processor_error_info_to_ir(EFI_IA32_X64_PROCESS_ERROR_INFO* error_info);
json_object* cper_ia32x64_cache_tlb_check_to_ir(EFI_IA32_X64_CACHE_CHECK_INFO* cache_tlb_check);
json_object* cper_ia32x64_bus_check_to_ir(EFI_IA32_X64_BUS_CHECK_INFO* bus_check);
json_object* cper_ia32x64_ms_check_to_ir(EFI_IA32_X64_MS_CHECK_INFO* ms_check);
json_object* cper_ia32x64_processor_context_info_to_ir(EFI_IA32_X64_PROCESSOR_CONTEXT_INFO* context_info, void** cur_pos);
json_object* cper_ia32x64_register_32bit_to_ir(EFI_CONTEXT_IA32_REGISTER_STATE* registers);
json_object* cper_ia32x64_register_64bit_to_ir(EFI_CONTEXT_X64_REGISTER_STATE* registers);

//Converts the IA32/x64 error section described in the given descriptor into intermediate format.
json_object* cper_section_ia32x64_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_IA32_X64_PROCESSOR_ERROR_RECORD* record = (EFI_IA32_X64_PROCESSOR_ERROR_RECORD*)section;
    json_object* record_ir = json_object_new_object();

    //Flags.
    json_object* flags = json_object_new_object();
    json_object_object_add(flags, "localAPICIDValid", json_object_new_boolean(record->ValidFields & 0b1));
    json_object_object_add(flags, "cpuIDInfoValid", json_object_new_boolean((record->ValidFields >> 1) & 0b1));
    int processor_error_info_num = (record->ValidFields >> 2) & 0b111111;
    json_object_object_add(flags, "processorErrorInfoNum", json_object_new_int(processor_error_info_num));
    int processor_context_info_num = (record->ValidFields >> 8) & 0b111111;
    json_object_object_add(flags, "processorContextInfoNum", json_object_new_int(processor_context_info_num));
    json_object_object_add(record_ir, "flags", flags);

    //APIC ID.
    json_object_object_add(record_ir, "localAPICID", json_object_new_uint64(record->ApicId));

    //CPUID information.
    json_object* cpuid_info_ir = json_object_new_object();
    EFI_IA32_X64_CPU_ID* cpuid_info = (EFI_IA32_X64_CPU_ID*)record->CpuIdInfo;
    json_object_object_add(cpuid_info_ir, "eax", json_object_new_uint64(cpuid_info->Eax));
    json_object_object_add(cpuid_info_ir, "ebx", json_object_new_uint64(cpuid_info->Ebx));
    json_object_object_add(cpuid_info_ir, "ecx", json_object_new_uint64(cpuid_info->Ecx));
    json_object_object_add(cpuid_info_ir, "edx", json_object_new_uint64(cpuid_info->Edx));
    json_object_object_add(record_ir, "cpuidInfo", cpuid_info_ir);

    //Processor error information, of the amount described above.
    EFI_IA32_X64_PROCESS_ERROR_INFO* current_error_info = (EFI_IA32_X64_PROCESS_ERROR_INFO*)(record + 1);
    json_object* error_info_array = json_object_new_array();
    for (int i=0; i<processor_error_info_num; i++) 
    {
        json_object_array_add(error_info_array, cper_ia32x64_processor_error_info_to_ir(current_error_info));
        current_error_info++;
    }
    json_object_object_add(record_ir, "processorErrorInfo", error_info_array);

    //Processor context information, of the amount described above.
    EFI_IA32_X64_PROCESSOR_CONTEXT_INFO* current_context_info = (EFI_IA32_X64_PROCESSOR_CONTEXT_INFO*)(current_error_info + 1);
    json_object* context_info_array = json_object_new_array();
    for (int i=0; i<processor_context_info_num; i++) 
    {
        json_object_array_add(context_info_array, cper_ia32x64_processor_context_info_to_ir(current_context_info, (void**)&current_context_info));
        //The context array is a non-fixed size, pointer is shifted within the above function.
    }
    json_object_object_add(record_ir, "processorContextInfo", context_info_array);

    return record_ir;
}

//Converts a single IA32/x64 processor error info block into JSON IR format.
json_object* cper_ia32x64_processor_error_info_to_ir(EFI_IA32_X64_PROCESS_ERROR_INFO* error_info)
{
    json_object* error_info_ir = json_object_new_object();

    //Error structure type (as GUID).
    char error_type[GUID_STRING_LENGTH];
    guid_to_string(error_type, &error_info->ErrorType);
    json_object_object_add(error_info_ir, "type", json_object_new_string(error_type));

    //Validation bits.
    json_object* validation = bitfield_to_ir(error_info->ValidFields, 5, IA32X64_PROCESSOR_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(error_info_ir, "validationBits", validation);

    //Add the check information on a per-structure basis.
    //Cache and TLB check information are identical, so can be equated.
    json_object* checkInformation = NULL;
    if (guid_equal(&error_info->ErrorType, &gEfiIa32x64ErrorTypeCacheCheckGuid)
        || guid_equal(&error_info->ErrorType, &gEfiIa32x64ErrorTypeTlbCheckGuid))
    {
        checkInformation = cper_ia32x64_cache_tlb_check_to_ir((EFI_IA32_X64_CACHE_CHECK_INFO*)&error_info->CheckInfo);
    }
    else if (guid_equal(&error_info->ErrorType, &gEfiIa32x64ErrorTypeBusCheckGuid))
        checkInformation = cper_ia32x64_bus_check_to_ir((EFI_IA32_X64_BUS_CHECK_INFO*)&error_info->CheckInfo);
    else if (guid_equal(&error_info->ErrorType, &gEfiIa32x64ErrorTypeMsCheckGuid))
        checkInformation = cper_ia32x64_ms_check_to_ir((EFI_IA32_X64_MS_CHECK_INFO*)&error_info->CheckInfo);
    json_object_object_add(error_info_ir, "checkInfo", checkInformation);

    //Target, requestor, and responder identifiers.
    json_object_object_add(error_info_ir, "targetIdentifier", json_object_new_uint64(error_info->TargetId));
    json_object_object_add(error_info_ir, "requestorIdentifier", json_object_new_uint64(error_info->RequestorId));
    json_object_object_add(error_info_ir, "responderIdentifier", json_object_new_uint64(error_info->ResponderId));
    json_object_object_add(error_info_ir, "instructionPointer", json_object_new_uint64(error_info->InstructionIP));

    return error_info_ir;
}

//Converts a single IA32/x64 cache or TLB check check info block into JSON IR format.
json_object* cper_ia32x64_cache_tlb_check_to_ir(EFI_IA32_X64_CACHE_CHECK_INFO* cache_tlb_check)
{
    json_object* cache_tlb_check_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(cache_tlb_check->ValidFields, 8, IA32X64_CHECK_INFO_VALID_BITFIELD_NAMES);
    json_object_object_add(cache_tlb_check_ir, "validationBits", validation);

    //Transaction type.
    json_object* transaction_type = integer_to_readable_pair(cache_tlb_check->TransactionType, 3,
        IA32X64_CHECK_INFO_TRANSACTION_TYPES_KEYS,
        IA32X64_CHECK_INFO_TRANSACTION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(cache_tlb_check_ir, "transactionType", transaction_type);

    //Operation.
    json_object* operation = integer_to_readable_pair(cache_tlb_check->Operation, 9,
        IA32X64_CHECK_INFO_OPERATION_TYPES_KEYS,
        IA32X64_CHECK_INFO_OPERATION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(cache_tlb_check_ir, "operation", operation);

    //Affected cache/TLB level.
    json_object_object_add(cache_tlb_check_ir, "level", json_object_new_uint64(cache_tlb_check->Level));

    //Miscellaneous boolean fields.
    json_object_object_add(cache_tlb_check_ir, "processorContextCorrupt", json_object_new_boolean(cache_tlb_check->ContextCorrupt));
    json_object_object_add(cache_tlb_check_ir, "uncorrected", json_object_new_boolean(cache_tlb_check->ErrorUncorrected));
    json_object_object_add(cache_tlb_check_ir, "preciseIP", json_object_new_boolean(cache_tlb_check->PreciseIp));
    json_object_object_add(cache_tlb_check_ir, "restartableIP", json_object_new_boolean(cache_tlb_check->RestartableIp));
    json_object_object_add(cache_tlb_check_ir, "overflow", json_object_new_boolean(cache_tlb_check->Overflow));

    return cache_tlb_check_ir;
}

//Converts a single IA32/x64 bus check check info block into JSON IR format.
json_object* cper_ia32x64_bus_check_to_ir(EFI_IA32_X64_BUS_CHECK_INFO* bus_check)
{ 
    json_object* bus_check_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(bus_check->ValidFields, 11, IA32X64_CHECK_INFO_VALID_BITFIELD_NAMES);
    json_object_object_add(bus_check_ir, "validationBits", validation);

    //Transaction type.
    json_object* transaction_type = integer_to_readable_pair(bus_check->TransactionType, 3,
        IA32X64_CHECK_INFO_TRANSACTION_TYPES_KEYS,
        IA32X64_CHECK_INFO_TRANSACTION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(bus_check_ir, "transactionType", transaction_type);

    //Operation.
    json_object* operation = integer_to_readable_pair(bus_check->Operation, 9,
        IA32X64_CHECK_INFO_OPERATION_TYPES_KEYS,
        IA32X64_CHECK_INFO_OPERATION_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(bus_check_ir, "operation", operation);

    //Affected bus level.
    json_object_object_add(bus_check_ir, "level", json_object_new_uint64(bus_check->Level));

    //Miscellaneous boolean fields.
    json_object_object_add(bus_check_ir, "processorContextCorrupt", json_object_new_boolean(bus_check->ContextCorrupt));
    json_object_object_add(bus_check_ir, "uncorrected", json_object_new_boolean(bus_check->ErrorUncorrected));
    json_object_object_add(bus_check_ir, "preciseIP", json_object_new_boolean(bus_check->PreciseIp));
    json_object_object_add(bus_check_ir, "restartableIP", json_object_new_boolean(bus_check->RestartableIp));
    json_object_object_add(bus_check_ir, "overflow", json_object_new_boolean(bus_check->Overflow));
    json_object_object_add(bus_check_ir, "timedOut", json_object_new_boolean(bus_check->TimeOut));

    //Participation type.
    json_object* participation_type = integer_to_readable_pair(bus_check->ParticipationType, 4,
        IA32X64_BUS_CHECK_INFO_PARTICIPATION_TYPES_KEYS,
        IA32X64_BUS_CHECK_INFO_PARTICIPATION_TYPES_VALUES,
        "Unknown");
    json_object_object_add(bus_check_ir, "participationType", participation_type);

    //Address space.
    json_object* address_space = integer_to_readable_pair(bus_check->AddressSpace, 4,
        IA32X64_BUS_CHECK_INFO_ADDRESS_SPACE_TYPES_KEYS,
        IA32X64_BUS_CHECK_INFO_ADDRESS_SPACE_TYPES_VALUES,
        "Unknown");
    json_object_object_add(bus_check_ir, "addressSpace", address_space);
    
    return bus_check_ir;
}

//Converts a single IA32/x64 MS check check info block into JSON IR format.
json_object* cper_ia32x64_ms_check_to_ir(EFI_IA32_X64_MS_CHECK_INFO* ms_check)
{
    json_object* ms_check_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(ms_check->ValidFields, 6, IA32X64_CHECK_INFO_VALID_BITFIELD_NAMES);
    json_object_object_add(ms_check_ir, "validationBits", validation);

    //Error type (operation that caused the error).
    json_object* error_type = integer_to_readable_pair(ms_check->ErrorType, 4,
        IA32X64_MS_CHECK_INFO_ERROR_TYPES_KEYS,
        IA32X64_MS_CHECK_INFO_ERROR_TYPES_VALUES,
        "Unknown (Processor Specific)");
    json_object_object_add(ms_check_ir, "errorType", error_type);
    
    //Miscellaneous fields.
    json_object_object_add(ms_check_ir, "processorContextCorrupt", json_object_new_boolean(ms_check->ContextCorrupt));
    json_object_object_add(ms_check_ir, "uncorrected", json_object_new_boolean(ms_check->ErrorUncorrected));
    json_object_object_add(ms_check_ir, "preciseIP", json_object_new_boolean(ms_check->PreciseIp));
    json_object_object_add(ms_check_ir, "restartableIP", json_object_new_boolean(ms_check->RestartableIp));
    json_object_object_add(ms_check_ir, "overflow", json_object_new_boolean(ms_check->Overflow));

    return ms_check_ir;
}

//Converts a single IA32/x64 processor context info entry into JSON IR format.
json_object* cper_ia32x64_processor_context_info_to_ir(EFI_IA32_X64_PROCESSOR_CONTEXT_INFO* context_info, void** cur_pos)
{
    json_object* context_info_ir = json_object_new_object();

    //Register context type.
    json_object* context_type = integer_to_readable_pair(context_info->RegisterType, 8,
        IA32X64_REGISTER_CONTEXT_TYPES_KEYS,
        IA32X64_REGISTER_CONTEXT_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(context_info_ir, "registerContextType", context_type);

    //Register array size, MSR and MM address.
    json_object_object_add(context_info_ir, "registerArraySize", json_object_new_uint64(context_info->ArraySize));
    json_object_object_add(context_info_ir, "msrAddress", json_object_new_uint64(context_info->MsrAddress));
    json_object_object_add(context_info_ir, "mmRegisterAddress", json_object_new_uint64(context_info->MmRegisterAddress));

    //Register array.
    json_object* register_array = NULL;
    if (context_info->RegisterType == 2) 
    {
        EFI_CONTEXT_IA32_REGISTER_STATE* register_state = (EFI_CONTEXT_IA32_REGISTER_STATE*)(context_info + 1);
        register_array = cper_ia32x64_register_32bit_to_ir(register_state);
        *cur_pos = (void*)(register_state + 1);
    }
    else if (context_info->RegisterType == 3) 
    {
        EFI_CONTEXT_X64_REGISTER_STATE* register_state = (EFI_CONTEXT_X64_REGISTER_STATE*)(context_info + 1);
        register_array = cper_ia32x64_register_64bit_to_ir(register_state);
        *cur_pos = (void*)(register_state + 1);
    }
    else 
    {
        //No parseable data, just dump as base64 and shift the head to the next item.
        *cur_pos = (void*)(context_info + 1);

        char* encoded = b64_encode((unsigned char*)cur_pos, context_info->ArraySize);
        register_array = json_object_new_object();
        json_object_object_add(register_array, "data", json_object_new_string(encoded));
        free(encoded);

        *cur_pos = (void*)(((char*)*cur_pos) + context_info->ArraySize);
    }
    json_object_object_add(context_info_ir, "registerArray", register_array);

    return context_info_ir;
}

//Converts a single CPER IA32 register state into JSON IR format.
json_object* cper_ia32x64_register_32bit_to_ir(EFI_CONTEXT_IA32_REGISTER_STATE* registers)
{
    json_object* ia32_registers = json_object_new_object();
    json_object_object_add(ia32_registers, "eax", json_object_new_int(registers->Eax));
    json_object_object_add(ia32_registers, "ebx", json_object_new_int(registers->Ebx));
    json_object_object_add(ia32_registers, "ecx", json_object_new_int(registers->Ecx));
    json_object_object_add(ia32_registers, "edx", json_object_new_int(registers->Edx));
    json_object_object_add(ia32_registers, "esi", json_object_new_int(registers->Esi));
    json_object_object_add(ia32_registers, "edi", json_object_new_int(registers->Edi));
    json_object_object_add(ia32_registers, "ebp", json_object_new_int(registers->Ebp));
    json_object_object_add(ia32_registers, "esp", json_object_new_int(registers->Esp));
    json_object_object_add(ia32_registers, "cs", json_object_new_int(registers->Cs));
    json_object_object_add(ia32_registers, "ds", json_object_new_int(registers->Ds));
    json_object_object_add(ia32_registers, "ss", json_object_new_int(registers->Ss));
    json_object_object_add(ia32_registers, "es", json_object_new_int(registers->Es));
    json_object_object_add(ia32_registers, "fs", json_object_new_int(registers->Fs));
    json_object_object_add(ia32_registers, "gs", json_object_new_int(registers->Gs));
    json_object_object_add(ia32_registers, "eflags", json_object_new_int(registers->Eflags));
    json_object_object_add(ia32_registers, "eip", json_object_new_int(registers->Eip));
    json_object_object_add(ia32_registers, "cr0", json_object_new_int(registers->Cr0));
    json_object_object_add(ia32_registers, "cr1", json_object_new_int(registers->Cr1));
    json_object_object_add(ia32_registers, "cr2", json_object_new_int(registers->Cr2));
    json_object_object_add(ia32_registers, "cr3", json_object_new_int(registers->Cr3));
    json_object_object_add(ia32_registers, "cr4", json_object_new_int(registers->Cr4));
    json_object_object_add(ia32_registers, "gdtr", json_object_new_uint64((registers->Gdtr[0] << 16) + registers->Gdtr[1]));
    json_object_object_add(ia32_registers, "idtr", json_object_new_uint64((registers->Idtr[0] << 16) + registers->Idtr[1]));
    json_object_object_add(ia32_registers, "ldtr", json_object_new_int(registers->Ldtr));
    json_object_object_add(ia32_registers, "tr", json_object_new_int(registers->Tr));

    return ia32_registers;
}

//Converts a single CPER x64 register state into JSON IR format.
json_object* cper_ia32x64_register_64bit_to_ir(EFI_CONTEXT_X64_REGISTER_STATE* registers)
{
    json_object* x64_registers = json_object_new_object();
    json_object_object_add(x64_registers, "rax", json_object_new_uint64(registers->Rax));
    json_object_object_add(x64_registers, "rbx", json_object_new_uint64(registers->Rbx));
    json_object_object_add(x64_registers, "rcx", json_object_new_uint64(registers->Rcx));
    json_object_object_add(x64_registers, "rdx", json_object_new_uint64(registers->Rdx));
    json_object_object_add(x64_registers, "rsi", json_object_new_uint64(registers->Rsi));
    json_object_object_add(x64_registers, "rdi", json_object_new_uint64(registers->Rdi));
    json_object_object_add(x64_registers, "rbp", json_object_new_uint64(registers->Rbp));
    json_object_object_add(x64_registers, "rsp", json_object_new_uint64(registers->Rsp));
    json_object_object_add(x64_registers, "r8", json_object_new_uint64(registers->R8));
    json_object_object_add(x64_registers, "r9", json_object_new_uint64(registers->R9));
    json_object_object_add(x64_registers, "r10", json_object_new_uint64(registers->R10));
    json_object_object_add(x64_registers, "r11", json_object_new_uint64(registers->R11));
    json_object_object_add(x64_registers, "r12", json_object_new_uint64(registers->R12));
    json_object_object_add(x64_registers, "r13", json_object_new_uint64(registers->R13));
    json_object_object_add(x64_registers, "r14", json_object_new_uint64(registers->R14));
    json_object_object_add(x64_registers, "r15", json_object_new_uint64(registers->R15));
    json_object_object_add(x64_registers, "cs", json_object_new_int(registers->Cs));
    json_object_object_add(x64_registers, "ds", json_object_new_int(registers->Ds));
    json_object_object_add(x64_registers, "ss", json_object_new_int(registers->Ss));
    json_object_object_add(x64_registers, "es", json_object_new_int(registers->Es));
    json_object_object_add(x64_registers, "fs", json_object_new_int(registers->Fs));
    json_object_object_add(x64_registers, "gs", json_object_new_int(registers->Gs));
    json_object_object_add(x64_registers, "rflags", json_object_new_uint64(registers->Rflags));
    json_object_object_add(x64_registers, "eip", json_object_new_uint64(registers->Rip));
    json_object_object_add(x64_registers, "cr0", json_object_new_uint64(registers->Cr0));
    json_object_object_add(x64_registers, "cr1", json_object_new_uint64(registers->Cr1));
    json_object_object_add(x64_registers, "cr2", json_object_new_uint64(registers->Cr2));
    json_object_object_add(x64_registers, "cr3", json_object_new_uint64(registers->Cr3));
    json_object_object_add(x64_registers, "cr4", json_object_new_uint64(registers->Cr4));
    json_object_object_add(x64_registers, "cr8", json_object_new_uint64(registers->Cr8));
    json_object_object_add(x64_registers, "gdtr_0", json_object_new_uint64(registers->Gdtr[0]));
    json_object_object_add(x64_registers, "gdtr_1", json_object_new_uint64(registers->Gdtr[1]));
    json_object_object_add(x64_registers, "idtr_0", json_object_new_uint64(registers->Idtr[0]));
    json_object_object_add(x64_registers, "idtr_1", json_object_new_uint64(registers->Idtr[1]));
    json_object_object_add(x64_registers, "ldtr", json_object_new_int(registers->Ldtr));
    json_object_object_add(x64_registers, "tr", json_object_new_int(registers->Tr));

    return x64_registers;
}