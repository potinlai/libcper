/**
 * Describes functions for converting CXL protocol error CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdio.h>
#include "json.h"
#include "b64.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-cxl-protocol.h"

//Converts a single CXL protocol error CPER section into JSON IR.
json_object* cper_section_cxl_protocol_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_CXL_PROTOCOL_ERROR_DATA* cxl_protocol_error = (EFI_CXL_PROTOCOL_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = bitfield_to_ir(cxl_protocol_error->ValidBits, 7, CXL_PROTOCOL_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Type of detecting agent.
    json_object* agent_type = integer_to_readable_pair(cxl_protocol_error->CxlAgentType, 2,
        CXL_PROTOCOL_ERROR_AGENT_TYPES_KEYS,
        CXL_PROTOCOL_ERROR_AGENT_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "agentType", agent_type);

    //CXL agent address, depending on the agent type.
    json_object* agent_address = json_object_new_object();
    if (cxl_protocol_error->CxlAgentType == CXL_PROTOCOL_ERROR_DEVICE_AGENT)
    {
        //Address is a CXL1.1 device agent.
        json_object_object_add(agent_address, "functionNumber", 
            json_object_new_uint64(cxl_protocol_error->CxlAgentAddress.DeviceAddress.FunctionNumber));
        json_object_object_add(agent_address, "deviceNumber", 
            json_object_new_uint64(cxl_protocol_error->CxlAgentAddress.DeviceAddress.DeviceNumber));
        json_object_object_add(agent_address, "busNumber", 
            json_object_new_uint64(cxl_protocol_error->CxlAgentAddress.DeviceAddress.BusNumber));
        json_object_object_add(agent_address, "segmentNumber", 
            json_object_new_uint64(cxl_protocol_error->CxlAgentAddress.DeviceAddress.SegmentNumber));
    }
    else if (cxl_protocol_error->CxlAgentType == CXL_PROTOCOL_ERROR_HOST_DOWNSTREAM_PORT_AGENT)
    {
        //Address is a CXL port RCRB base address.
        json_object_object_add(agent_address, "value", 
            json_object_new_uint64(cxl_protocol_error->CxlAgentAddress.PortRcrbBaseAddress));
    }
    json_object_object_add(section_ir, "cxlAgentAddress", agent_address);

    //Device ID.
    json_object* device_id = json_object_new_object();
    json_object_object_add(device_id, "vendorID", 
        json_object_new_uint64(cxl_protocol_error->DeviceId.VendorId));
    json_object_object_add(device_id, "deviceID", 
        json_object_new_uint64(cxl_protocol_error->DeviceId.DeviceId));
    json_object_object_add(device_id, "subsystemVendorID", 
        json_object_new_uint64(cxl_protocol_error->DeviceId.SubsystemVendorId));
    json_object_object_add(device_id, "subsystemDeviceID", 
        json_object_new_uint64(cxl_protocol_error->DeviceId.SubsystemDeviceId));
    json_object_object_add(device_id, "classCode", 
        json_object_new_uint64(cxl_protocol_error->DeviceId.ClassCode));
    json_object_object_add(device_id, "slotNumber", 
        json_object_new_uint64(cxl_protocol_error->DeviceId.SlotNumber));
    json_object_object_add(section_ir, "deviceID", device_id);

    //Device serial & capability structure (if CXL 1.1 device).
    if (cxl_protocol_error->CxlAgentType == CXL_PROTOCOL_ERROR_DEVICE_AGENT)
    {
        json_object_object_add(section_ir, "deviceSerial", json_object_new_uint64(cxl_protocol_error->DeviceSerial));

        //The PCIe capability structure provided here could either be PCIe 1.1 Capability Structure 
        //(36-byte, padded to 60 bytes) or PCIe 2.0 Capability Structure (60-byte). There does not seem
        //to be a way to differentiate these, so this is left as a b64 dump.
        char* encoded = b64_encode(cxl_protocol_error->CapabilityStructure.PcieCap, 60);
        json_object_object_add(section_ir, "capabilityStructure", json_object_new_uint64(cxl_protocol_error->DeviceSerial));
        free(encoded);
    }

    //CXL DVSEC & error log length.
    json_object_object_add(section_ir, "dvsecLength", json_object_new_int(cxl_protocol_error->CxlDvsecLength));
    json_object_object_add(section_ir, "errorLogLength", json_object_new_int(cxl_protocol_error->CxlErrorLogLength));

    //CXL DVSEC
    //For CXL 1.1 devices, this is the "CXL DVSEC For Flex Bus Device" structure as in CXL 1.1 spec.
    //For CXL 1.1 host downstream ports, this is the "CXL DVSEC For Flex Bus Port" structure as in CXL 1.1 spec.
    unsigned char* cur_pos = (unsigned char*)(cxl_protocol_error + 1);
    char* encoded = b64_encode(cur_pos, cxl_protocol_error->CxlDvsecLength);
    json_object_object_add(section_ir, "cxlDVSEC", json_object_new_string(encoded));
    free(encoded);
    cur_pos += cxl_protocol_error->CxlDvsecLength;

    //CXL Error Log
    //This is the "CXL RAS Capability Structure" as in CXL 1.1 spec.
    encoded = b64_encode(cur_pos, cxl_protocol_error->CxlErrorLogLength);
    json_object_object_add(section_ir, "cxlErrorLog", json_object_new_string(encoded));
    free(encoded);
    
    return section_ir;
}