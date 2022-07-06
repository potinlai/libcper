/**
 * Describes functions for converting Intel IPF CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-ipf.h"

//Converts a single Intel IPF error CPER section into JSON IR.
json_object* cper_section_ipf_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    //... todo ...
}