/**
 * Describes functions for converting memory error CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-memory.h"

//Converts a single memory error CPER section into JSON IR.
json_object* cper_section_platform_memory_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_PLATFORM_MEMORY_ERROR_DATA* memory_error = (EFI_PLATFORM_MEMORY_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Validation bitfield.
    json_object* validation = bitfield_to_ir(memory_error->ValidFields, 22, MEMORY_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Error status.
    json_object* error_status = cper_generic_error_status_to_ir(&memory_error->ErrorStatus);
    json_object_object_add(section_ir, "errorStatus", error_status);

    //Miscellaneous numeric fields.
    json_object_object_add(section_ir, "physicalAddress", json_object_new_uint64(memory_error->PhysicalAddress));
    json_object_object_add(section_ir, "physicalAddressMask", json_object_new_uint64(memory_error->PhysicalAddressMask));
    json_object_object_add(section_ir, "node", json_object_new_uint64(memory_error->Node));
    json_object_object_add(section_ir, "card", json_object_new_uint64(memory_error->Card));
    json_object_object_add(section_ir, "moduleRank", json_object_new_uint64(memory_error->ModuleRank));
    json_object_object_add(section_ir, "device", json_object_new_uint64(memory_error->Device));
    json_object_object_add(section_ir, "row", json_object_new_uint64(memory_error->Row));
    json_object_object_add(section_ir, "column", json_object_new_uint64(memory_error->Column));
    json_object_object_add(section_ir, "bitPosition", json_object_new_uint64(memory_error->BitPosition));
    json_object_object_add(section_ir, "requestorID", json_object_new_uint64(memory_error->RequestorId));
    json_object_object_add(section_ir, "responderID", json_object_new_uint64(memory_error->ResponderId));
    json_object_object_add(section_ir, "targetID", json_object_new_uint64(memory_error->TargetId));
    json_object_object_add(section_ir, "rankNumber", json_object_new_uint64(memory_error->RankNum));
    json_object_object_add(section_ir, "cardSmbiosHandle", json_object_new_uint64(memory_error->CardHandle));
    json_object_object_add(section_ir, "moduleSmbiosHandle", json_object_new_uint64(memory_error->ModuleHandle));

    //Bank.
    json_object* bank = json_object_new_object();
    json_object_object_add(bank, "address", json_object_new_uint64(memory_error->Bank & 0xFF));
    json_object_object_add(bank, "group", json_object_new_uint64(memory_error->Bank >> 8));
    json_object_object_add(section_ir, "bank", bank);

    //Memory error type.
    json_object* memory_error_type = integer_to_readable_pair(memory_error->ErrorType, 16,
        MEMORY_ERROR_TYPES_KEYS,
        MEMORY_ERROR_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "memoryErrorType", memory_error_type);

    //"Extended" row/column indication field + misc.
    json_object* extended = json_object_new_object();
    json_object_object_add(extended, "rowBit16", json_object_new_boolean(memory_error->Extended & 0b1));
    json_object_object_add(extended, "rowBit17", json_object_new_boolean((memory_error->Extended >> 1) & 0b1));
    json_object_object_add(extended, "chipIdentification", json_object_new_int(memory_error->Extended >> 5));
    json_object_object_add(section_ir, "extended", extended);

    return section_ir;
}

//Converts a single memory error 2 CPER section into JSON IR.
json_object* cper_section_platform_memory2_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_PLATFORM_MEMORY2_ERROR_DATA* memory_error = (EFI_PLATFORM_MEMORY2_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //... todo

    return section_ir;
}