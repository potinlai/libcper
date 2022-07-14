#ifndef CPER_SECTION_DMAR_IOMMU_H
#define CPER_SECTION_DMAR_IOMMU_H

#include "json.h"
#include "../edk/Cper.h"

json_object* cper_section_dmar_iommu_to_ir(void* section, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);
void ir_section_dmar_iommu_to_cper(json_object* section, FILE* out);

#endif