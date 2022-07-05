#ifndef CPER_SECTION_ARM_H
#define CPER_SECTION_ARM_H

#include "json.h"
#include "../edk/Cper.h"

#define ARM_PROCESSOR_ERROR_VALID_BITFIELD_NAMES (const char*[]) \
    {"mpidrValid", "errorAffinityLevelValid", "runningStateValid", "vendorSpecificInfoValid"}
    
json_object* cper_section_arm_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);

#endif