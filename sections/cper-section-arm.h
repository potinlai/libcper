#ifndef CPER_SECTION_ARM_H
#define CPER_SECTION_ARM_H

#include "json.h"
#include "../edk/Cper.h"

#define ARM_PROCESSOR_ERROR_VALID_BITFIELD_NAMES (const char*[]) \
    {"mpidrValid", "errorAffinityLevelValid", "runningStateValid", "vendorSpecificInfoValid"}
#define ARM_PROCESSOR_ERROR_INFO_ENTRY_VALID_BITFIELD_NAMES (const char*[]) \
    {"multipleErrorValid", "flagsValid", "errorInformationValid", "virtualFaultAddressValid", "physicalFaultAddressValid"}
#define ARM_PROCESSOR_ERROR_INFO_ENTRY_FLAGS_NAMES (const char*[]) \
    {"firstErrorCaptured", "lastErrorCaptured", "propagated", "overflow"}
#define ARM_PROCESSOR_ERROR_INFO_ENTRY_INFO_TYPES_KEYS (int []){0, 1, 2, 3}
#define ARM_PROCESSOR_ERROR_INFO_ENTRY_INFO_TYPES_VALUES (const char*[]){"Cache Error", "TLB Error", \
    "Bus Error", "Micro-Architectural Error"}

json_object* cper_section_arm_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);

#endif