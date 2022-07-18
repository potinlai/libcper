#ifndef GEN_UTILS_H
#define GEN_UTILS_H

#include <stdlib.h>
#include "../edk/BaseTypes.h"

size_t generate_random_section(void** location, size_t size);
UINT8* generate_random_bytes(size_t size);
void init_random();

#endif