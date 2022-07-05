/**
 * Describes functions for converting processor-generic CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-generic.h"

//Converts the given processor-generic CPER section into JSON IR.
json_object* cper_section_generic_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_PROCESSOR_GENERIC_ERROR_DATA* section_generic = (EFI_PROCESSOR_GENERIC_ERROR_DATA*)section;
    json_object* section_ir = json_object_new_object();

    //Validation bits.
    json_object* validation = json_object_new_object();
    json_object_object_add(validation, "processorTypeValid", json_object_new_boolean(section_generic->ValidFields >> 63));
    json_object_object_add(validation, "processorISAValid", json_object_new_boolean((section_generic->ValidFields >> 62) & 0x1));
    json_object_object_add(validation, "processorErrorTypeValid", json_object_new_boolean((section_generic->ValidFields >> 61) & 0x1));
    json_object_object_add(validation, "operationValid", json_object_new_boolean((section_generic->ValidFields >> 60) & 0x1));
    json_object_object_add(validation, "flagsValid", json_object_new_boolean((section_generic->ValidFields >> 59) & 0x1));
    json_object_object_add(validation, "levelValid", json_object_new_boolean((section_generic->ValidFields >> 58) & 0x1));
    json_object_object_add(validation, "cpuVersionValid", json_object_new_boolean((section_generic->ValidFields >> 57) & 0x1));
    json_object_object_add(validation, "cpuBrandInfoValid", json_object_new_boolean((section_generic->ValidFields >> 56) & 0x1));
    json_object_object_add(validation, "cpuIDValid", json_object_new_boolean((section_generic->ValidFields >> 55) & 0x1));
    json_object_object_add(validation, "targetAddressValid", json_object_new_boolean((section_generic->ValidFields >> 54) & 0x1));
    json_object_object_add(validation, "requesterIDValid", json_object_new_boolean((section_generic->ValidFields >> 53) & 0x1));
    json_object_object_add(validation, "responderIDValid", json_object_new_boolean((section_generic->ValidFields >> 52) & 0x1));
    json_object_object_add(validation, "instructionIPValid", json_object_new_boolean((section_generic->ValidFields >> 51) & 0x1));
    json_object_object_add(section_ir, "validationBits", validation);

    //Processor type, with human readable name if possible.
    json_object* processor_type = integer_to_readable_pair(section_generic->Type, 
        sizeof(GENERIC_PROC_TYPES_KEYS) / sizeof(int),
        GENERIC_PROC_TYPES_KEYS,
        GENERIC_PROC_TYPES_VALUES,
        "Unknown (Reserved)");
    json_object_object_add(section_ir, "processorType", processor_type);

    //Processor ISA, with human readable name if possible.
    json_object* processor_isa = integer_to_readable_pair(section_generic->Isa, 
        sizeof(GENERIC_ISA_TYPES_KEYS) / sizeof(int),
        GENERIC_ISA_TYPES_KEYS,
        GENERIC_ISA_TYPES_VALUES,
        "Unknown (Reserved");
    json_object_object_add(section_ir, "processorISA", processor_isa);

    //Processor error type, with human readable name if possible.
    json_object* processor_error_type = integer_to_readable_pair(section_generic->ErrorType, 
        sizeof(GENERIC_ERROR_TYPES_KEYS) / sizeof(int),
        GENERIC_ERROR_TYPES_KEYS, 
        GENERIC_ERROR_TYPES_VALUES, 
        "Unknown (Reserved");
    json_object_object_add(section_ir, "errorType", processor_error_type);

    //The operation performed, with a human readable name if possible.
    json_object* operation = integer_to_readable_pair(section_generic->Operation, 
        sizeof(GENERIC_OPERATION_TYPES_KEYS) / sizeof(int),
        GENERIC_OPERATION_TYPES_KEYS, 
        GENERIC_OPERATION_TYPES_VALUES, 
        "Unknown (Reserved");
    json_object_object_add(section_ir, "operation", operation);

    //Flags, additional information about the error.
    json_object* flags = json_object_new_object();
    json_object_object_add(flags, "restartable", json_object_new_boolean(section_generic->Flags >> 7));
    json_object_object_add(flags, "preciseIP", json_object_new_boolean((section_generic->Flags >> 6) & 0b1));
    json_object_object_add(flags, "overflow", json_object_new_boolean((section_generic->Flags >> 5) & 0b1));
    json_object_object_add(flags, "corrected", json_object_new_boolean((section_generic->Flags >> 4) & 0b1));
    json_object_object_add(section_ir, "flags", flags);

    //The level of the error.
    json_object_object_add(section_ir, "level", json_object_new_int(section_generic->Level));

    //CPU version information (todo)
    //...

    //CPU brand string. May not exist if on ARM.
    json_object_object_add(section_ir, "cpuBrandString", json_object_new_string(section_generic->BrandString));

    //Remaining 64-bit fields.
    json_object_object_add(section_ir, "processorID", json_object_new_uint64(section_generic->ApicId));
    json_object_object_add(section_ir, "targetAddress", json_object_new_uint64(section_generic->TargetAddr));
    json_object_object_add(section_ir, "requestorID", json_object_new_uint64(section_generic->RequestorId));
    json_object_object_add(section_ir, "responderID", json_object_new_uint64(section_generic->ResponderId));
    json_object_object_add(section_ir, "instructionIP", json_object_new_uint64(section_generic->InstructionIP));

    return section_ir;
}