/**
 * Describes functions for converting CXL component error CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-cxl-component.h"

//Converts a single CXL component error CPER section into JSON IR.
json_object* cper_section_cxl_component_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    //The length of the structure (bytes).
    EFI_CXL_COMPONENT_EVENT_HEADER* cxl_error = (EFI_CXL_COMPONENT_EVENT_HEADER*)section;
    json_object* section_ir = json_object_new_object();
    
    //todo: Figure out structure for CXL Component Event log (UEFI spec is a bit vague)

    return section_ir;
}