/**
 * Describes functions for converting CCIX PER log CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "b64.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-ccix-per.h"

//Converts a single CCIX PER log CPER section into JSON IR.
json_object* cper_section_ccix_per_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_CCIX_PER_LOG_DATA* ccix_error = (EFI_CCIX_PER_LOG_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Length (bytes) for the entire structure.
    json_object_object_add(section_ir, "length", json_object_new_uint64(ccix_error->Length));

    //Validation bits.
    json_object* validation = bitfield_to_ir(ccix_error->ValidBits, 3, CCIX_PER_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //CCIX source/port IDs.
    json_object_object_add(section_ir, "ccixSourceID", json_object_new_int(ccix_error->CcixSourceId));
    json_object_object_add(section_ir, "ccixPortID", json_object_new_int(ccix_error->CcixPortId));
    
    //CCIX PER Log.
    //This is formatted as described in Section 7.3.2 of CCIX Base Specification (Rev 1.0).
    unsigned char* cur_pos = (unsigned char*)(ccix_error + 1);
    int remaining_length = section - (void*)cur_pos + ccix_error->Length;
    if (remaining_length > 0)
    {
        char* encoded = b64_encode(cur_pos, remaining_length);
        json_object_object_add(section_ir, "ccixPERLog", json_object_new_string(encoded));
        free(encoded);
    }

    return section_ir;
}