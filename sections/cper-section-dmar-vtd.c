/**
 * Describes functions for converting VT-d specific DMAr CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
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

    //Fault record, root and context entry.
    //todo: Look at the VT-d specification and implement

    //PTE entry for all page levels.
    json_object_object_add(section_ir, "pageTableEntry_Level6", json_object_new_uint64(vtd_error->PteL6));
    json_object_object_add(section_ir, "pageTableEntry_Level5", json_object_new_uint64(vtd_error->PteL5));
    json_object_object_add(section_ir, "pageTableEntry_Level4", json_object_new_uint64(vtd_error->PteL4));
    json_object_object_add(section_ir, "pageTableEntry_Level3", json_object_new_uint64(vtd_error->PteL3));
    json_object_object_add(section_ir, "pageTableEntry_Level2", json_object_new_uint64(vtd_error->PteL2));
    json_object_object_add(section_ir, "pageTableEntry_Level1", json_object_new_uint64(vtd_error->PteL1));

    return section_ir;
}