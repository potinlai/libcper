#ifndef CPER_UTILS_H
#define CPER_UTILS_H

#define GUID_STRING_LENGTH 30
#define TIMESTAMP_LENGTH 24

json_object* integer_to_readable_pair(int value, int len, int keys[], const char* values[], const char* default_value);
json_object* bitfield_to_ir(UINT64 bitfield, int num_fields, const char* names[]);
json_object* revision_to_ir(UINT16 revision);
const char* severity_to_string(UINT8 severity);
void guid_to_string(char* out, EFI_GUID* guid);
int guid_equal(EFI_GUID* a, EFI_GUID* b);

//The available severity types for CPER.
extern const char* CPER_SEVERITY_TYPES[4];

#endif