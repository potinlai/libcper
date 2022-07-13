#ifndef CPER_UTILS_H
#define CPER_UTILS_H

#define GUID_STRING_LENGTH 30
#define TIMESTAMP_LENGTH 24

json_object* cper_generic_error_status_to_ir(EFI_GENERIC_ERROR_STATUS* error_status);
json_object* uniform_struct_to_ir(UINT32* start, int len, const char* names[]);
json_object* uniform_struct64_to_ir(UINT64* start, int len, const char* names[]);
json_object* integer_to_readable_pair(UINT64 value, int len, int keys[], const char* values[], const char* default_value);
json_object* integer_to_readable_pair_with_desc(int value, int len, int keys[], const char* values[], const char* descriptions[], const char* default_value);
UINT64 readable_pair_to_integer(json_object* pair);
json_object* bitfield_to_ir(UINT64 bitfield, int num_fields, const char* names[]);
UINT64 ir_to_bitfield(json_object* ir, int num_fields, const char* names[]);
json_object* uint64_array_to_ir_array(UINT64* array, int len);
json_object* revision_to_ir(UINT16 revision);
const char* severity_to_string(UINT8 severity);
void timestamp_to_string(char* out, EFI_ERROR_TIME_STAMP* timestamp);
void string_to_timestamp(EFI_ERROR_TIME_STAMP* out, const char* timestamp);
void guid_to_string(char* out, EFI_GUID* guid);
void string_to_guid(EFI_GUID* out, const char* guid);
int guid_equal(EFI_GUID* a, EFI_GUID* b);
int bcd_to_int(UINT8 bcd);

//The available severity types for CPER.
extern const char* CPER_SEVERITY_TYPES[4];

#endif