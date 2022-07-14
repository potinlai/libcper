/**
 * Describes functions for converting PCIe CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include <string.h>
#include "json.h"
#include "b64.h"
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
    UINT64 class_id = (pcie_error->DevBridge.ClassCode[0] << 16) + 
                      (pcie_error->DevBridge.ClassCode[1] << 8) +
                      pcie_error->DevBridge.ClassCode[2];
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
    //The PCIe capability structure provided here could either be PCIe 1.1 Capability Structure 
    //(36-byte, padded to 60 bytes) or PCIe 2.0 Capability Structure (60-byte). There does not seem
    //to be a way to differentiate these, so this is left as a b64 dump.
    char* encoded = b64_encode((unsigned char*)pcie_error->Capability.PcieCap, 60);
    json_object* capability = json_object_new_object();
    json_object_object_add(capability, "data", json_object_new_string(encoded));
    free(encoded);
    json_object_object_add(section_ir, "capabilityStructure", capability);

    //AER information.
    json_object* aer_capability_ir = json_object_new_object();
    EFI_PCIE_ADV_ERROR_EXT_CAPABILITY* aer_capability = (EFI_PCIE_ADV_ERROR_EXT_CAPABILITY*)pcie_error->AerInfo.PcieAer;
    json_object_object_add(aer_capability_ir, "capabilityID", 
        json_object_new_uint64(aer_capability->Header.PcieExtendedCapabilityId));
    json_object_object_add(aer_capability_ir, "capabilityVersion", 
        json_object_new_uint64(aer_capability->Header.CapabilityVersion));
    json_object_object_add(aer_capability_ir, "uncorrectableErrorStatusRegister", 
        json_object_new_uint64(aer_capability->UncorrectableErrorStatusReg));
    json_object_object_add(aer_capability_ir, "uncorrectableErrorMaskRegister", 
        json_object_new_uint64(aer_capability->UncorrectableErrorMaskReg));
    json_object_object_add(aer_capability_ir, "uncorrectableErrorSeverityRegister", 
        json_object_new_uint64(aer_capability->UncorrectableErrorSeverityReg));
    json_object_object_add(aer_capability_ir, "correctableErrorStatusRegister", 
        json_object_new_uint64(aer_capability->CorrectableErrorStatusReg));
    json_object_object_add(aer_capability_ir, "correctableErrorMaskRegister", 
        json_object_new_uint64(aer_capability->CorrectableErrorMaskReg));
    json_object_object_add(aer_capability_ir, "aeccReg", json_object_new_uint64(aer_capability->AeccReg));

    //Header log register (b64).
    encoded = b64_encode((unsigned char*)aer_capability->HeaderLogReg, 16);
    json_object_object_add(aer_capability_ir, "headerLogRegister", json_object_new_string(encoded));
    free(encoded);

    //Remaining AER fields.
    json_object_object_add(aer_capability_ir, "rootErrorCommand", 
        json_object_new_uint64(aer_capability->RootErrorCommand));
    json_object_object_add(aer_capability_ir, "rootErrorStatus", 
        json_object_new_uint64(aer_capability->RootErrorStatus));
    json_object_object_add(aer_capability_ir, "errorSourceIDRegister", 
        json_object_new_uint64(aer_capability->ErrorSourceIdReg));
    json_object_object_add(aer_capability_ir, "correctableErrorSourceIDRegister", 
        json_object_new_uint64(aer_capability->CorrectableSourceIdReg));

    json_object_object_add(section_ir, "aerInfo", aer_capability_ir);
    return section_ir;
}

//Converts a single CPER-JSON PCIe section into CPER binary, outputting to the given stream.
void ir_section_pcie_to_cper(json_object* section, FILE* out)
{
    EFI_PCIE_ERROR_DATA* section_cper = (EFI_PCIE_ERROR_DATA*)calloc(1, sizeof(EFI_PCIE_ERROR_DATA));

    //Validation bits.
    section_cper->ValidFields = ir_to_bitfield(json_object_object_get(section, "validationBits"), 
        8, PCIE_ERROR_VALID_BITFIELD_NAMES);

    //Version.
    json_object* version = json_object_object_get(section, "version");
    int minor = json_object_get_int(json_object_object_get(version, "minor"));
    int major = json_object_get_int(json_object_object_get(version, "major"));
    section_cper->Version = int_to_bcd(minor) + ((UINT16)(int_to_bcd(major)) << 8);

    //Command/status registers.
    json_object* command_status = json_object_object_get(section, "commandStatus");
    UINT32 command = (UINT16)json_object_get_uint64(json_object_object_get(command_status, "commandRegister"));
    UINT32 status = (UINT16)json_object_get_uint64(json_object_object_get(command_status, "statusRegister"));
    section_cper->CommandStatus = command + (status << 16);

    //Device ID.
    json_object* device_id = json_object_object_get(section, "deviceID");
    UINT64 class_id = json_object_get_uint64(json_object_object_get(device_id, "classCode"));
    section_cper->DevBridge.VendorId = 
        (UINT16)json_object_get_uint64(json_object_object_get(device_id, "vendorID"));
    section_cper->DevBridge.DeviceId = 
        (UINT16)json_object_get_uint64(json_object_object_get(device_id, "deviceID"));
    section_cper->DevBridge.ClassCode[0] = class_id >> 16;
    section_cper->DevBridge.ClassCode[1] = (class_id >> 8) & 0xFF;
    section_cper->DevBridge.ClassCode[1] = class_id & 0xFF;
    section_cper->DevBridge.Function = 
        (UINT8)json_object_get_uint64(json_object_object_get(device_id, "functionNumber"));
    section_cper->DevBridge.Device = 
        (UINT8)json_object_get_uint64(json_object_object_get(device_id, "deviceNumber"));
    section_cper->DevBridge.Segment = 
        (UINT16)json_object_get_uint64(json_object_object_get(device_id, "segmentNumber"));
    section_cper->DevBridge.PrimaryOrDeviceBus = 
        (UINT8)json_object_get_uint64(json_object_object_get(device_id, "primaryOrDeviceBusNumber"));
    section_cper->DevBridge.SecondaryBus = 
        (UINT8)json_object_get_uint64(json_object_object_get(device_id, "secondaryBusNumber"));
    section_cper->DevBridge.Slot.Number = 
        (UINT16)json_object_get_uint64(json_object_object_get(device_id, "slotNumber"));

    //Bridge/control status.
    json_object* bridge_control = json_object_object_get(section, "bridgeControlStatus");
    UINT32 bridge_status = (UINT16)json_object_get_uint64(json_object_object_get(bridge_control, "secondaryStatusRegister"));
    UINT32 control_status = (UINT16)json_object_get_uint64(json_object_object_get(bridge_control, "controlRegister"));
    section_cper->BridgeControlStatus = bridge_status + (control_status << 16);

    //Capability structure.
    json_object* capability = json_object_object_get(section, "capabilityStructure");
    json_object* encoded = json_object_object_get(capability, "data");
    UINT8* decoded = b64_decode(json_object_get_string(encoded), json_object_get_string_len(encoded));
    memcpy(section_cper->Capability.PcieCap, decoded, 60);
    free(decoded);

    //AER capability structure.
    json_object* aer_info = json_object_object_get(section, "aerInfo");
    EFI_PCIE_ADV_ERROR_EXT_CAPABILITY* aer_capability = 
        (EFI_PCIE_ADV_ERROR_EXT_CAPABILITY*)section_cper->AerInfo.PcieAer;
    aer_capability->Header.PcieExtendedCapabilityId =
        json_object_get_uint64(json_object_object_get(aer_info, "capabilityID"));
    aer_capability->Header.CapabilityVersion =
        json_object_get_uint64(json_object_object_get(aer_info, "capabilityVersion"));
    aer_capability->UncorrectableErrorStatusReg =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "uncorrectableErrorStatusRegister"));
    aer_capability->UncorrectableErrorMaskReg =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "uncorrectableErrorMaskRegister"));
    aer_capability->UncorrectableErrorSeverityReg =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "uncorrectableErrorSeverityRegister"));
    aer_capability->CorrectableErrorStatusReg =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "correctableErrorStatusRegister"));
    aer_capability->CorrectableErrorMaskReg =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "correctableErrorMaskRegister"));
    aer_capability->AeccReg =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "aeccReg"));

    //AER header log register.
    encoded = json_object_object_get(aer_info, "headerLogRegister");
    decoded = b64_decode(json_object_get_string(encoded), json_object_get_string_len(encoded));
    memcpy(aer_capability->HeaderLogReg, decoded, 16);
    free(decoded);

    //Remaining AER fields.
    aer_capability->RootErrorCommand =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "rootErrorCommand"));
    aer_capability->RootErrorStatus =
        (UINT32)json_object_get_uint64(json_object_object_get(aer_info, "rootErrorStatus"));
    aer_capability->ErrorSourceIdReg =
        (UINT16)json_object_get_uint64(json_object_object_get(aer_info, "errorSourceIDRegister"));
    aer_capability->CorrectableSourceIdReg =
        (UINT16)json_object_get_uint64(json_object_object_get(aer_info, "correctableErrorSourceIDRegister"));


    //Miscellaneous value fields.
    section_cper->PortType = (UINT32)readable_pair_to_integer(json_object_object_get(section, "portType"));
    section_cper->SerialNo = json_object_get_uint64(json_object_object_get(section, "deviceSerialNumber"));

    //Write out to stream, free resources.
    fwrite(&section_cper, sizeof(section_cper), 1, out);
    fflush(out);
    free(section_cper);
}