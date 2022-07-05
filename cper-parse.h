#ifndef CPER_PARSE_H
#define CPER_PARSE_H
#include "json.h"

#define CPER_HEADER_FLAG_TYPES_KEYS (int []){1, 2, 3}
#define CPER_HEADER_FLAG_TYPES_VALUES (const char*[]){"HW_ERROR_FLAGS_RECOVERED", "HW_ERROR_FLAGS_PREVERR", "HW_ERROR_FLAGS_SIMULATED"}

json_object* cper_to_ir(const char* filename);

#endif