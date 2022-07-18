/**
 * Utility functions to assist in generating psuedo-random CPER sections. 
 * 
 * Author: Lawrence.Tang@arm.com
 **/
#include <stdlib.h>
#include <time.h>
#include "../edk/BaseTypes.h"
#include "gen-utils.h"

//Generates a random section of the given byte size, saving the result to the given location.
//Returns the length of the section as passed in.
size_t generate_random_section(void** location, size_t size)
{
    *location = generate_random_bytes(size);
    return size;
}

//Generates a random byte allocation of the given size.
UINT8* generate_random_bytes(size_t size)
{
    UINT8* bytes = malloc(size);
    for (size_t i = 0; i < size; i++)
        bytes[i] = rand();
    return bytes;
}

//Initializes the random seed for rand() using the current time.
void init_random()
{
    srand((unsigned int)time(NULL));
}