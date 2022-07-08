#ifndef CPER_PARSE_H
#define CPER_PARSE_H
#include "json.h"

#define CPER_HEADER_VALID_BITFIELD_NAMES (const char*[]) {"platformIDValid", "timestampValid", "partitionIDValid"}
#define CPER_SECTION_DESCRIPTOR_FLAGS_BITFIELD_NAMES (const char*[]) \
    {"primary", "containmentWarning", "reset", "errorThresholdExceeded", "resourceNotAccessible", "latentError", \
    "propagated", "overflow"}
#define CPER_HEADER_FLAG_TYPES_KEYS (int []){1, 2, 3}
#define CPER_HEADER_FLAG_TYPES_VALUES (const char*[]){"HW_ERROR_FLAGS_RECOVERED", "HW_ERROR_FLAGS_PREVERR", "HW_ERROR_FLAGS_SIMULATED"}

json_object* cper_to_ir(FILE* cper_file);
void ir_to_cper(json_object* ir, FILE* out);

#endif