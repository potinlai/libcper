/**
 * Describes functions for converting processor-generic CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"

//Converts the given processor-generic CPER section into JSON IR.
json_object* cper_section_generic_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    return NULL; //todo
}