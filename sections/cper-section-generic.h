#ifndef CPER_SECTION_GENERIC_H
#define CPER_SECTION_GENERIC_H

#define GENERIC_PROC_TYPES_KEYS (int []){0, 1, 2}
#define GENERIC_PROC_TYPES_VALUES (const char*[]){"IA32/X64", "IA64", "ARM"}
#define GENERIC_ISA_TYPES_KEYS (int []){0, 1, 2, 3, 4}
#define GENERIC_ISA_TYPES_VALUES (const char*[]){"IA32", "IA64", "X64", "ARM A32/T32", "ARM A64"}
#define GENERIC_ERROR_TYPES_KEYS (int []){0, 1, 2, 4, 8}
#define GENERIC_ERROR_TYPES_VALUES (const char*[]){"Unknown", "Cache Error", "TLB Error", "Bus Error", "Micro-Architectural Error"}
#define GENERIC_OPERATION_TYPES_KEYS (int []){0, 1, 2, 3}
#define GENERIC_OPERATION_TYPES_VALUES (const char*[]){"Unknown or generic", "Data Read", "Data Write", "Instruction Execution"}

json_object* cper_section_generic_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);

#endif