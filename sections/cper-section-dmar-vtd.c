/**
 * Describes functions for converting VT-d specific DMAr CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "b64.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-dmar-vtd.h"

//Converts a single VT-d specific DMAr CPER section into JSON IR.
json_object* cper_section_dmar_vtd_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_DIRECTED_IO_DMAR_ERROR_DATA* vtd_error = (EFI_DIRECTED_IO_DMAR_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Version, revision and OEM ID, as defined in the VT-d architecture.
    UINT64 oem_id = (vtd_error->OemId[0] << 16) + (vtd_error->OemId[1] << 8) + vtd_error->OemId[2];
    json_object_object_add(section_ir, "version", json_object_new_int(vtd_error->Version));
    json_object_object_add(section_ir, "revision", json_object_new_int(vtd_error->Revision));
    json_object_object_add(section_ir, "oemID", json_object_new_uint64(oem_id));

    //Registers.
    json_object_object_add(section_ir, "capabilityRegister", json_object_new_uint64(vtd_error->Capability));
    json_object_object_add(section_ir, "extendedCapabilityRegister", json_object_new_uint64(vtd_error->CapabilityEx));
    json_object_object_add(section_ir, "globalCommandRegister", json_object_new_uint64(vtd_error->GlobalCommand));
    json_object_object_add(section_ir, "globalStatusRegister", json_object_new_uint64(vtd_error->GlobalStatus));
    json_object_object_add(section_ir, "faultStatusRegister", json_object_new_uint64(vtd_error->FaultStatus));

    //Fault record basic fields.
    json_object* fault_record_ir = json_object_new_object();
    EFI_VTD_FAULT_RECORD* fault_record = (EFI_VTD_FAULT_RECORD*)vtd_error->FaultRecord;
    json_object_object_add(fault_record_ir, "faultInformation", json_object_new_uint64(fault_record->FaultInformation));
    json_object_object_add(fault_record_ir, "sourceIdentifier", json_object_new_uint64(fault_record->SourceIdentifier));
    json_object_object_add(fault_record_ir, "privelegeModeRequested", 
        json_object_new_boolean(fault_record->PrivelegeModeRequested));
    json_object_object_add(fault_record_ir, "executePermissionRequested", 
        json_object_new_boolean(fault_record->ExecutePermissionRequested));
    json_object_object_add(fault_record_ir, "pasidPresent", json_object_new_boolean(fault_record->PasidPresent));
    json_object_object_add(fault_record_ir, "faultReason", json_object_new_uint64(fault_record->FaultReason));
    json_object_object_add(fault_record_ir, "pasidValue", json_object_new_uint64(fault_record->PasidValue));
    json_object_object_add(fault_record_ir, "addressType", json_object_new_uint64(fault_record->AddressType));

    //Fault record address type.
    json_object* fault_record_type = integer_to_readable_pair(fault_record->Type, 2,
        VTD_FAULT_RECORD_TYPES_KEYS,
        VTD_FAULT_RECORD_TYPES_VALUES,
        "Unknown");
    json_object_object_add(fault_record_ir, "type", fault_record_type);
    json_object_object_add(section_ir, "faultRecord", fault_record_ir);

    //Root entry.
    char* encoded = b64_encode((unsigned char*)vtd_error->RootEntry, 16);
    json_object_object_add(section_ir, "rootEntry", json_object_new_string(encoded));
    free(encoded);

    //Context entry.
    encoded = b64_encode((unsigned char*)vtd_error->ContextEntry, 16);
    json_object_object_add(section_ir, "contextEntry", json_object_new_string(encoded));
    free(encoded);

    //PTE entry for all page levels.
    json_object_object_add(section_ir, "pageTableEntry_Level6", json_object_new_uint64(vtd_error->PteL6));
    json_object_object_add(section_ir, "pageTableEntry_Level5", json_object_new_uint64(vtd_error->PteL5));
    json_object_object_add(section_ir, "pageTableEntry_Level4", json_object_new_uint64(vtd_error->PteL4));
    json_object_object_add(section_ir, "pageTableEntry_Level3", json_object_new_uint64(vtd_error->PteL3));
    json_object_object_add(section_ir, "pageTableEntry_Level2", json_object_new_uint64(vtd_error->PteL2));
    json_object_object_add(section_ir, "pageTableEntry_Level1", json_object_new_uint64(vtd_error->PteL1));

    return section_ir;
}