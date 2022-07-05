/**
 * Describes functions for converting ARM CPER sections from binary and JSON format
 * into an intermediate format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "../edk/Cper.h"
#include "../cper-utils.h"
#include "cper-section-arm.h"

//Converts the given processor-generic CPER section into JSON IR.
json_object* cper_section_arm_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    EFI_ARM_PROCESSOR_ERROR_RECORD* record = (EFI_ARM_PROCESSOR_ERROR_RECORD*)section;
    json_object* section_ir = json_object_new_object();
    
    //Validation bits.
    json_object* validation = bitfield_to_ir(record->ValidFields, 4, ARM_PROCESSOR_ERROR_VALID_BITFIELD_NAMES);
    json_object_object_add(section_ir, "validationBits", validation);

    //Number of error info and context info structures, and length.
    json_object_object_add(section_ir, "errorInfoNum", json_object_new_int(record->ErrInfoNum));
    json_object_object_add(section_ir, "contextInfoNum", json_object_new_int(record->ContextInfoNum));
    json_object_object_add(section_ir, "sectionLength", json_object_new_int(record->SectionLength));

    //Error affinity.
    json_object* error_affinity = json_object_new_object();
    json_object_object_add(error_affinity, "value", json_object_new_int(record->ErrorAffinityLevel));
    json_object_object_add(error_affinity, "type", 
        json_object_new_string(record->ErrorAffinityLevel < 4 ? "Vendor Defined" : "Reserved"));
    json_object_object_add(section_ir, "errorAffinity", error_affinity);

    //Processor ID (MPIDR_EL1) and chip ID (MIDR_EL1).
    json_object_object_add(section_ir, "mpidrEl1", json_object_new_uint64(record->MPIDR_EL1));
    json_object_object_add(section_ir, "midrEl1", json_object_new_uint64(record->MIDR_EL1));

    //Whether the processor is running, and the state of it if so.
    json_object_object_add(section_ir, "running", json_object_new_boolean(record->RunningState));
    if (record->RunningState)
    {
        //...       
    }

    return section_ir;
}