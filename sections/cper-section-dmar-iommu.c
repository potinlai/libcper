/**
 * Describes functions for converting IOMMU specific DMAr CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "b64.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-dmar-iommu.h"

//Converts a single IOMMU specific DMAr CPER section into JSON IR.
json_object* cper_section_dmar_iommu_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_IOMMU_DMAR_ERROR_DATA* iommu_error = (EFI_IOMMU_DMAR_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Revision.
    json_object_object_add(section_ir, "revision", json_object_new_int(iommu_error->Revision));

    //IOMMU registers.
    json_object_object_add(section_ir, "controlRegister", json_object_new_uint64(iommu_error->Control));
    json_object_object_add(section_ir, "statusRegister", json_object_new_uint64(iommu_error->Status));

    //IOMMU event log entry.
    //The format of these entries differ widely by the type of error.
    char* encoded = b64_encode((unsigned char*)iommu_error->EventLogEntry, 16);
    json_object_object_add(section_ir, "eventLogEntry", json_object_new_string(encoded));
    free(encoded);

    //Device table entry (as base64).
    encoded = b64_encode((unsigned char*)iommu_error->DeviceTableEntry, 32);
    json_object_object_add(section_ir, "deviceTableEntry", json_object_new_string(encoded));
    free(encoded);

    //Page table entries.
    json_object_object_add(section_ir, "pageTableEntry_Level6", json_object_new_uint64(iommu_error->PteL6));
    json_object_object_add(section_ir, "pageTableEntry_Level5", json_object_new_uint64(iommu_error->PteL5));
    json_object_object_add(section_ir, "pageTableEntry_Level4", json_object_new_uint64(iommu_error->PteL4));
    json_object_object_add(section_ir, "pageTableEntry_Level3", json_object_new_uint64(iommu_error->PteL3));
    json_object_object_add(section_ir, "pageTableEntry_Level2", json_object_new_uint64(iommu_error->PteL2));
    json_object_object_add(section_ir, "pageTableEntry_Level1", json_object_new_uint64(iommu_error->PteL1));

    return section_ir;
}