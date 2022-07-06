#ifndef CPER_SECTION_ARM_H
#define CPER_SECTION_ARM_H

#include "json.h"
#include "../edk/Cper.h"

#define MEMORY_ERROR_VALID_BITFIELD_NAMES (const char*[]) \
    {"errorStatusValid", "physicalAddressValid", "physicalAddressMaskValid", "nodeValid", "cardValid", "moduleValid", \
    "bankValid", "deviceValid", "rowValid", "columnValid", "bitPositionValid", "platformRequestorIDValid", \
    "platformResponderIDValid", "memoryPlatformTargetValid", "memoryErrorTypeValid", "rankNumberValid", \
    "cardHandleValid", "moduleHandleValid", "extendedRowBitsValid", "bankGroupValid", "bankAddressValid", \
    "chipIdentificationValid"}
#define MEMORY_ERROR_ERROR_TYPES_KEYS (int []){1, 16, 4, 5, 6, 7, 8, 9, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}
#define MEMORY_ERROR_ERROR_TYPES_VALUES (const char*[]){"ERR_INTERNAL", "ERR_BUS", "ERR_MEM", "ERR_TLB", \
    "ERR_CACHE", "ERR_FUNCTION", "ERR_SELFTEST", "ERR_FLOW", "ERR_MAP", "ERR_IMPROPER", "ERR_UNIMPL", \
    "ERR_LOL", "ERR_RESPONSE", "ERR_PARITY", "ERR_PROTOCOL", "ERR_ERROR", "ERR_TIMEOUT", "ERR_POISONED"}
#define MEMORY_ERROR_ERROR_TYPES_DESCRIPTIONS (const char*[]){\
    "Error detected internal to the component.", \
    "Error detected in the bus.", \
    "Storage error in memory (DRAM).", \
    "Storage error in TLB.", \
    "Storage error in cache.", \
    "Error in one or more functional units.", \
    "Component failed self test.", \
    "Overflow or underflow of internal queue.", \
    "Virtual address not found on IO-TLB or IO-PDIR.", \
    "Improper access error.", \
    "Access to a memory address which is not mapped to any component.", \
    "Loss of Lockstep error.", \
    "Response not associated with a request.", \
    "Bus parity error (must also set the A, C, or D bits).", \
    "Detection of a protocol error.", \
    "Detection of a PATH_ERROR.", \
    "Bus operation timeout.", \
    "A read was issued to data that has been poisoned."}

json_object* cper_section_memory_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);

#endif