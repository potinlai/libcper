/**
 * Functions for generating pseudo-random CXL component error sections.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdlib.h>
#include "../../edk/BaseTypes.h"
#include "../gen-utils.h"
#include "gen-sections.h"

//Generates a single pseudo-random CXL component error section, saving the resulting address to the given
//location. Returns the size of the newly created section.
size_t generate_section_cxl_component(void** location)
{
    //Create a random length for the CXL component event log.
    //The logs attached here do not necessarily conform to the specification, and are simply random.
    int log_len = rand() % 64;

    //Create random bytes.
    int size = 32 + log_len;
    UINT8* bytes = generate_random_bytes(size);

    //Set reserved areas to zero.
    UINT64* validation = (UINT64*)(bytes + 4);
    *validation &= 0b111;
    UINT16* slot_number = (UINT16*)(bytes + 21);
    *slot_number &= ~0b111; //Device ID slot number bits 0-2.
    *(bytes + 23) = 0; //Device ID byte 11.

    //Set expected values.
    UINT32* length = (UINT32*)bytes;
    *length = size;
    
    //Set return values, exit.
    *location = bytes;
    return size;
}