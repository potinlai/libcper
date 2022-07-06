/**
 * Describes functions for converting PCI/PCI-X bus CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-pci-bus.h"

//Converts a single PCI/PCI-X bus CPER section into JSON IR.
json_object* cper_section_pci_bus_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_PCI_PCIX_BUS_ERROR_DATA* bus_error = (EFI_PCI_PCIX_BUS_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(bus_error->ValidFields, 9, PCI_BUS_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Error status.
    json_object* error_status = cper_generic_error_status_to_ir(&bus_error->ErrorStatus);
    json_object_object_add(section_ir, "errorStatus", error_status);

    //PCI bus error type.
    json_object* error_type = integer_to_readable_pair(bus_error->Type, 8,
        PCI_BUS_ERROR_TYPES_KEYS,
        PCI_BUS_ERROR_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "errorType", error_type);

    //Bus ID.
    json_object* bus_id = json_object_new_object();
    json_object_object_add(bus_id, "busNumber", json_object_new_int(bus_error->BusId & 0xFF));
    json_object_object_add(bus_id, "segmentNumber", json_object_new_int(bus_error->BusId >> 8));
    json_object_object_add(section_ir, "busID", bus_id);

    //Miscellaneous numeric fields.
    json_object_object_add(section_ir, "busAddress", json_object_new_uint64(bus_error->BusAddress));
    json_object_object_add(section_ir, "busData", json_object_new_uint64(bus_error->BusData));
    json_object_object_add(section_ir, "busCommandType", json_object_new_string(bus_error->BusCommand == 0 ? "PCI" : "PCI-X"));
    json_object_object_add(section_ir, "busRequestorID", json_object_new_uint64(bus_error->RequestorId));
    json_object_object_add(section_ir, "busCompleterID", json_object_new_uint64(bus_error->ResponderId));
    json_object_object_add(section_ir, "targetID", json_object_new_uint64(bus_error->TargetId));

    return section_ir;
}