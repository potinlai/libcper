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