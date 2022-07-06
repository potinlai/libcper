#ifndef CPER_SECTION_DMAR_VTD_H
#define CPER_SECTION_DMAR_VTD_H

#include "json.h"
#include "../edk/Cper.h"

json_object* cper_section_dmar_vtd_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);

#endif