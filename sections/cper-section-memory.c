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
json_object* cper_section_memory_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_PLATFORM_MEMORY_ERROR_DATA* memory_error = (EFI_PLATFORM_MEMORY_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Validation bitfield.
    json_object* validation = bitfield_to_ir(memory_error->ValidFields, 22, MEMORY_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Error status.
    json_object* error_status = json_object_new_object();
    json_object_object_add(error_status, "errorType", integer_to_readable_pair_with_desc(memory_error->ErrorStatus.Type, 18,
        MEMORY_ERROR_ERROR_TYPES_KEYS,
        MEMORY_ERROR_ERROR_TYPES_VALUES,
        MEMORY_ERROR_ERROR_TYPES_DESCRIPTIONS,
        "Unknown (Reserved)"));
}