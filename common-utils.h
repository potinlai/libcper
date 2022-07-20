#ifndef CPER_LIB_COMMON_UTILS_H
#define CPER_LIB_COMMON_UTILS_H

#include "edk/BaseTypes.h"

int bcd_to_int(UINT8 bcd);
UINT8 int_to_bcd(int value);

#endif