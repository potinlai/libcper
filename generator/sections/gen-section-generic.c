/**
 * Functions for generating psuedo-random CPER generic processor sections.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdlib.h>
#include "../../edk/BaseTypes.h"
#include "../gen-utils.h"
#include "gen-sections.h"

//Generates a single psuedo-random generic processor section, saving the resulting address to the given
//location. Returns the size of the newly created section.
size_t generate_section_generic(void** location)
{
    //Create random bytes.
    size_t size = generate_random_section(location, 192);

    //Set reserved locations to zero.
    UINT8* start_byte = (UINT8*)*location;
    *((UINT64*)start_byte) &= 0xFFF;
    *(start_byte + 12) &= 0b111;
    *((UINT16*)(start_byte + 14)) = 0x0;
    
    return size;
}