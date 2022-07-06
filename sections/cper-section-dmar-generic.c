/**
 * Describes functions for converting generic DMAr CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-dmar-generic.h"

//Converts a single generic DMAr CPER section into JSON IR.
json_object* cper_section_dmar_generic_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_DMAR_GENERIC_ERROR_DATA* firmware_error = (EFI_DMAR_GENERIC_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Requester ID, segment.
    json_object_object_add(section_ir, "requesterID", json_object_new_int(firmware_error->RequesterId));
    json_object_object_add(section_ir, "segmentNumber", json_object_new_int(firmware_error->SegmentNumber));

    //Fault reason.
    json_object* fault_reason = integer_to_readable_pair_with_desc(firmware_error->FaultReason, 11,
        DMAR_GENERIC_ERROR_FAULT_REASON_TYPES_KEYS,
        DMAR_GENERIC_ERROR_FAULT_REASON_TYPES_VALUES,
        DMAR_GENERIC_ERROR_FAULT_REASON_TYPES_DESCRIPTIONS,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "faultReason", fault_reason);

    //Access type.
    json_object* access_type = integer_to_readable_pair(firmware_error->AccessType, 2,
        DMAR_GENERIC_ERROR_ACCESS_TYPES_KEYS,
        DMAR_GENERIC_ERROR_ACCESS_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "accessType", access_type);

    //Address type.
    json_object* address_type = integer_to_readable_pair(firmware_error->AddressType, 2,
        DMAR_GENERIC_ERROR_ADDRESS_TYPES_KEYS,
        DMAR_GENERIC_ERROR_ADDRESS_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "addressType", address_type);

    //Architecture type.
    json_object* arch_type = integer_to_readable_pair(firmware_error->ArchType, 2,
        DMAR_GENERIC_ERROR_ARCH_TYPES_KEYS,
        DMAR_GENERIC_ERROR_ARCH_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "architectureType", arch_type);

    //Device address.
    json_object_object_add(section_ir, "deviceAddress", json_object_new_uint64(firmware_error->DeviceAddr));
    
    return section_ir;
}