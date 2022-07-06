/**
 * Describes utility functions for parsing CPER into JSON IR. 
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "edk/Cper.h"
#include "cper-utils.h"

//The available severity types for CPER.
const char* CPER_SEVERITY_TYPES[4] = {"Recoverable", "Fatal", "Corrected", "Informational"};

//Converts the given generic CPER error status to JSON IR.
json_object* cper_generic_error_status_to_ir(EFI_GENERIC_ERROR_STATUS* error_status)
{
    json_object* error_status_ir = json_object_new_object();

    //Error type.
    json_object_object_add(error_status_ir, "errorType", integer_to_readable_pair_with_desc(error_status->Type, 18,
        CPER_GENERIC_ERROR_TYPES_KEYS,
        CPER_GENERIC_ERROR_TYPES_VALUES,
        CPER_GENERIC_ERROR_TYPES_DESCRIPTIONS,
        "Unknown (Reserved)"));

    //Boolean bit fields.
    json_object_object_add(error_status_ir, "addressSignal", json_object_new_boolean(error_status->AddressSignal));
    json_object_object_add(error_status_ir, "controlSignal", json_object_new_boolean(error_status->ControlSignal));
    json_object_object_add(error_status_ir, "dataSignal", json_object_new_boolean(error_status->DataSignal));
    json_object_object_add(error_status_ir, "detectedByResponder", json_object_new_boolean(error_status->DetectedByResponder));
    json_object_object_add(error_status_ir, "detectedByRequester", json_object_new_boolean(error_status->DetectedByRequester));
    json_object_object_add(error_status_ir, "firstError", json_object_new_boolean(error_status->FirstError));
    json_object_object_add(error_status_ir, "overflowNotLogged", json_object_new_boolean(error_status->OverflowNotLogged));
    
    return error_status_ir;
}

//Converts a single uniform struct of UINT64s into intermediate JSON IR format, given names for each field in byte order.
json_object* uniform_struct64_to_ir(UINT64* start, int len, const char* names[])
{
    json_object* result = json_object_new_object();

    UINT64* cur = start;
    for (int i=0; i<len; i++)
    {
        json_object_object_add(result, names[i], json_object_new_uint64(*cur));
        cur++;
    }

    return result;
}

//Converts a single uniform struct of UINT32s into intermediate JSON IR format, given names for each field in byte order.
json_object* uniform_struct_to_ir(UINT32* start, int len, const char* names[])
{
    json_object* result = json_object_new_object();

    UINT32* cur = start;
    for (int i=0; i<len; i++)
    {
        json_object_object_add(result, names[i], json_object_new_uint64(*cur));
        cur++;
    }

    return result;
}

//Converts a single integer value to an object containing a value, and a readable name if possible.
json_object* integer_to_readable_pair(int value, int len, int keys[], const char* values[], const char* default_value)
{
    json_object* result = json_object_new_object();
    json_object_object_add(result, "value", json_object_new_int(value));

    //Search for human readable name, add.
    const char* name = default_value;
    for (int i=0; i<len; i++)
    {
        if (keys[i] == value)
            name = values[i];
    }

    json_object_object_add(result, "name", json_object_new_string(name));
    return result;
}

//Converts a single integer value to an object containing a value, readable name and description if possible.
json_object* integer_to_readable_pair_with_desc(int value, int len, int keys[], const char* values[], 
    const char* descriptions[], const char* default_value) 
{
    json_object* result = json_object_new_object();
    json_object_object_add(result, "value", json_object_new_int(value));

    //Search for human readable name, add.
    const char* name = default_value;
    for (int i=0; i<len; i++)
    {
        if (keys[i] == value) 
        {
            name = values[i];
            json_object_object_add(result, "description", json_object_new_string(descriptions[i]));
        }
    }

    json_object_object_add(result, "name", json_object_new_string(name));
    return result;
}

//Converts the given 64 bit bitfield to IR, assuming bit 0 starts on the left.
json_object* bitfield_to_ir(UINT64 bitfield, int num_fields, const char* names[])
{
    json_object* result = json_object_new_object();
    for (int i=0; i<num_fields; i++)
    {
        json_object_object_add(result, names[i], json_object_new_boolean((bitfield >> i) & 0b1));
    }

    return result;
}


//Converts a single UINT16 revision number into JSON IR representation.
json_object* revision_to_ir(UINT16 revision)
{
    json_object* revision_info = json_object_new_object();
    json_object_object_add(revision_info, "major", json_object_new_int(revision >> 8));
    json_object_object_add(revision_info, "minor", json_object_new_int(revision & 0xFF));
    return revision_info;
}

//Returns the appropriate string for the given integer severity.
const char* severity_to_string(UINT8 severity)
{
    return severity < 4 ? CPER_SEVERITY_TYPES[severity] : "Unknown";
}

//Helper function to convert an EDK EFI GUID into a string for intermediate use.
void guid_to_string(char* out, EFI_GUID* guid)
{
    sprintf(out, "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x", 
        guid->Data1, 
        guid->Data2, 
        guid->Data3,
        guid->Data4[0],
        guid->Data4[1],
        guid->Data4[2],
        guid->Data4[3],
        guid->Data4[4],
        guid->Data4[5],
        guid->Data4[6],
        guid->Data4[7]);
}

//Returns one if two EFI GUIDs are equal, zero otherwise.
int guid_equal(EFI_GUID* a, EFI_GUID* b)
{
    //Check top base 3 components.
    if (a->Data1 != b->Data1
        || a->Data2 != b->Data2
        || a->Data3 != b->Data3) 
    {
        return 0;
    }

    //Check Data4 array for equality.
    for (int i=0; i<8; i++)
    {
        if (a->Data4[i] != b->Data4[i])
            return 0;
    }

    return 1;
}