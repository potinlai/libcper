#ifndef GEN_SECTIONS_H
#define GEN_SECTIONS_H

#include <stdlib.h>

size_t generate_section_generic(void** location);
size_t generate_section_ia32x64(void** location);
size_t generate_section_arm(void** location);
size_t generate_section_memory(void** location);
size_t generate_section_memory2(void** location);
size_t generate_section_pcie(void** location);
size_t generate_section_pci_bus(void** location);
size_t generate_section_pci_dev(void** location);
size_t generate_section_firmware(void** location);
size_t generate_section_dmar_generic(void** location);
size_t generate_section_dmar_vtd(void** location);
size_t generate_section_dmar_iommu(void** location);
size_t generate_section_ccix_per(void** location);
size_t generate_section_cxl_protocol(void** location);
size_t generate_section_cxl_component(void** location);

#endif