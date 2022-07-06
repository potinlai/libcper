/**
 * Describes functions for converting firmware CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-firmware.h"

//Converts a single firmware CPER section into JSON IR.
json_object* cper_section_firmware_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_FIRMWARE_ERROR_DATA* firmware_error = (EFI_FIRMWARE_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Record type.
    json_object* record_type = integer_to_readable_pair(firmware_error->ErrorType, 3,
        FIRMWARE_ERROR_RECORD_TYPES_KEYS,
        FIRMWARE_ERROR_RECORD_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "errorRecordType", record_type);

    //Revision, record identifier.
    json_object_object_add(section_ir, "revision", json_object_new_int(firmware_error->Revision));
    json_object_object_add(section_ir, "recordID", json_object_new_uint64(firmware_error->RecordId));
    
    //Record GUID.
    char record_id_guid[GUID_STRING_LENGTH];
    guid_to_string(record_id_guid, &firmware_error->RecordIdGuid);
    json_object_object_add(section_ir, "recordIDGUID", json_object_new_string(record_id_guid));

    return section_ir;
}