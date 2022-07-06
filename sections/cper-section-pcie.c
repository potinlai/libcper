/**
 * Describes functions for converting PCIe CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-pcie.h"

//Converts a single PCIe CPER section into JSON IR.
json_object* cper_section_pcie_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_PCIE_ERROR_DATA* pcie_error = (EFI_PCIE_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(pcie_error->ValidFields, 8, PCIE_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Port type.
    json_object* port_type = integer_to_readable_pair(pcie_error->PortType, 9,
        PCIE_ERROR_PORT_TYPES_KEYS,
        PCIE_ERROR_PORT_TYPES_VALUES,
        "Unknown");
    json_object_object_add(section_ir, "portType", port_type);

    //Version, provided each half in BCD.
    json_object* version = json_object_new_object();
    json_object_object_add(version, "minor", json_object_new_int(bcd_to_int(pcie_error->Version & 0xFF)));
    json_object_object_add(version, "major", json_object_new_int(bcd_to_int(pcie_error->Version >> 8)));
    json_object_object_add(section_ir, "version", version);

    //Command & status.
    json_object* command_status = json_object_new_object();
    json_object_object_add(command_status, "commandRegister", json_object_new_uint64(pcie_error->CommandStatus & 0xFFFF));
    json_object_object_add(command_status, "statusRegister", json_object_new_uint64(pcie_error->CommandStatus >> 16));
    json_object_object_add(section_ir, "commandStatus", command_status);

    //PCIe Device ID.
    json_object* device_id = json_object_new_object();
    UINT64 class_id =  pcie_error->DevBridge.ClassCode[0] + 
                      (pcie_error->DevBridge.ClassCode[1] << 8) +
                      (pcie_error->DevBridge.ClassCode[2] << 16);
    json_object_object_add(device_id, "vendorID", json_object_new_uint64(pcie_error->DevBridge.VendorId));
    json_object_object_add(device_id, "deviceID", json_object_new_uint64(pcie_error->DevBridge.DeviceId));
    json_object_object_add(device_id, "classCode", json_object_new_uint64(class_id));
    json_object_object_add(device_id, "functionNumber", json_object_new_uint64(pcie_error->DevBridge.Function));
    json_object_object_add(device_id, "deviceNumber", json_object_new_uint64(pcie_error->DevBridge.Device));
    json_object_object_add(device_id, "segmentNumber", json_object_new_uint64(pcie_error->DevBridge.Segment));
    json_object_object_add(device_id, "primaryOrDeviceBusNumber", json_object_new_uint64(pcie_error->DevBridge.PrimaryOrDeviceBus));
    json_object_object_add(device_id, "secondaryBusNumber", json_object_new_uint64(pcie_error->DevBridge.SecondaryBus));
    json_object_object_add(device_id, "slotNumber", json_object_new_uint64(pcie_error->DevBridge.Slot.Number));
    json_object_object_add(section_ir, "deviceID", device_id);

    //Device serial number.
    json_object_object_add(section_ir, "deviceSerialNumber", json_object_new_uint64(pcie_error->SerialNo));

    //Bridge control status.
    json_object* bridge_control_status = json_object_new_object();
    json_object_object_add(bridge_control_status, "secondaryStatusRegister", 
        json_object_new_uint64(pcie_error->BridgeControlStatus & 0xFFFF));
    json_object_object_add(bridge_control_status, "controlRegister", 
        json_object_new_uint64(pcie_error->BridgeControlStatus >> 16));
    json_object_object_add(section_ir, "bridgeControlStatus", bridge_control_status);

    //Capability structure.
    //todo: See Figure 6-9 of the PCIe 2.0 Base Specification to implement this

    //AER information.
    //todo: See the PCIe 2.0 Base Specification to implement this.

    return section_ir;
}