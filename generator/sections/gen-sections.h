#ifndef GEN_SECTIONS_H
#define GEN_SECTIONS_H

#include <stdlib.h>

size_t generate_section_generic(void** location);
size_t generate_section_ia32x64(void** location);
size_t generate_section_arm(void** location);

#endif