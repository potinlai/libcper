#ifndef CPER_SECTION_PCIE_H
#define CPER_SECTION_PCIE_H

#include "json.h"
#include "../edk/Cper.h"

#define PCIE_ERROR_VALID_BITFIELD_NAMES (const char*[]) {"portTypeValid", "versionValid", "commandStatusValid", \
    "deviceIDValid", "deviceSerialNumberValid", "bridgeControlStatusValid", "capabilityStructureStatusValid", \
    "aerInfoValid"}
#define PCIE_ERROR_PORT_TYPES_KEYS (int []){0, 1, 4, 5, 6, 7, 8, 9, 10}
#define PCIE_ERROR_PORT_TYPES_VALUES (const char*[]){"PCI Express End Point", "Legacy PCI End Point Device", \
    "Root Port", "Upstream Switch Port", "Downstream Switch Port", "PCI Express to PCI/PCI-X Bridge", \
    "PCI/PCI-X Bridge to PCI Express Bridge", "Root Complex Integrated Endpoint Device", "Root Complex Event Collector"}

typedef struct {
    UINT64 PcieExtendedCapabilityId : 16;
    UINT64 CapabilityVersion : 4;
    UINT64 NextCapabilityOffset : 12; 
} EFI_PCIE_ADV_ERROR_EXT_CAPABILITY_HEADER;

typedef struct {
    EFI_PCIE_ADV_ERROR_EXT_CAPABILITY_HEADER Header;
    UINT32 UncorrectableErrorStatusReg;
    UINT32 UncorrectableErrorMaskReg;
    UINT32 UncorrectableErrorSeverityReg;
    UINT32 CorrectableErrorStatusReg;
    UINT32 CorrectableErrorMaskReg;
    UINT32 AeccReg;
    UINT8 HeaderLogReg[16];
    UINT32 RootErrorCommand;
    UINT32 RootErrorStatus;
    UINT16 CorrectableSourceIdReg;
    UINT16 ErrorSourceIdReg;
} EFI_PCIE_ADV_ERROR_EXT_CAPABILITY;

json_object* cper_section_pcie_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);
void ir_section_pcie_to_cper(json_object* section, FILE* out);

#endif