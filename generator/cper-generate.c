/**
 * Describes functions for generating psuedo-random specification compliant CPER records. 
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../edk/Cper.h"
#include "gen-utils.h"
#include "sections/gen-sections.h"
#include "cper-generate.h"

EFI_ERROR_SECTION_DESCRIPTOR* generate_section_descriptor(char* type, size_t* lengths, int index, int num_sections);
size_t generate_section(void** location, char* type);

//Generates a CPER record with the given section types, outputting to the given stream.
void generate_cper_record(char** types, UINT16 num_sections, FILE* out)
{
    //Initialise randomiser.
    init_random();

    //Generate the sections.
    void* sections[num_sections];
    size_t section_lengths[num_sections];
    for (int i=0; i<num_sections; i++) 
    {
        section_lengths[i] = generate_section(sections + i, types[i]);
        if (section_lengths[i] == 0)
        {
            //Error encountered, exit.
            printf("Error encountered generating section %d of type '%s', length returned zero.\n", i+1, types[i]);
            return;
        }
    }

    //Generate the header given the number of sections.
    EFI_COMMON_ERROR_RECORD_HEADER* header = 
        (EFI_COMMON_ERROR_RECORD_HEADER*)calloc(1, sizeof(EFI_COMMON_ERROR_RECORD_HEADER));
    header->SignatureStart = 0x52455043; //CPER
    header->SectionCount = num_sections;
    header->SignatureEnd = 0xFFFFFFFF;
    header->Flags = 4; //HW_ERROR_FLAGS_SIMULATED
    header->RecordID = (UINT64)rand();
    header->ErrorSeverity = rand() % 4;

    //Generate a valid timestamp.
    header->TimeStamp.Century = int_to_bcd(rand() % 100);
    header->TimeStamp.Year = int_to_bcd(rand() % 100);
    header->TimeStamp.Month = int_to_bcd(rand() % 12 + 1);
    header->TimeStamp.Day = int_to_bcd(rand() % 31 + 1);
    header->TimeStamp.Hours = int_to_bcd(rand() % 24 + 1);
    header->TimeStamp.Seconds = int_to_bcd(rand() % 60);

    //Turn all validation bits on.
    header->ValidationBits = 0b11;

    //Generate the section descriptors given the number of sections.
    EFI_ERROR_SECTION_DESCRIPTOR* section_descriptors[num_sections];
    for (int i=0; i<num_sections; i++)
        section_descriptors[i] = generate_section_descriptor(types[i], section_lengths, i, num_sections);

    //Calculate total length of structure, set in header.
    size_t total_len = sizeof(EFI_COMMON_ERROR_RECORD_HEADER);
    for (int i=0; i<num_sections; i++)
        total_len += section_lengths[i];
    total_len += num_sections * sizeof(EFI_ERROR_SECTION_DESCRIPTOR);
    header->RecordLength = (UINT32)total_len;

    //Write to stream in order, free all resources.
    fwrite(header, sizeof(EFI_COMMON_ERROR_RECORD_HEADER), 1, out);
    fflush(out);
    free(header);
    for (int i=0; i<num_sections; i++)
    {
        fwrite(section_descriptors[i], sizeof(EFI_ERROR_SECTION_DESCRIPTOR), 1, out);
        fflush(out);
        free(section_descriptors[i]);
    }
    for (int i=0; i<num_sections; i++) 
    {
        fwrite(sections[i], section_lengths[i], 1, out);
        fflush(out);
        free(sections[i]);
    }
}

//Generates a single section descriptor for a section with the given properties.
EFI_ERROR_SECTION_DESCRIPTOR* generate_section_descriptor(char* type, size_t* lengths, int index, int num_sections)
{
    EFI_ERROR_SECTION_DESCRIPTOR* descriptor = 
        (EFI_ERROR_SECTION_DESCRIPTOR*)generate_random_bytes(sizeof(EFI_ERROR_SECTION_DESCRIPTOR));

    //Set reserved bits to zero.
    descriptor->Resv1 = 0;
    descriptor->SectionFlags &= 0xFF;

    //Validation bits all set to 'on'.
    descriptor->SecValidMask = 0b11;

    //Set severity.
    descriptor->Severity = rand() % 4;

    //Set length, offset from base record.
    descriptor->SectionLength = (UINT32)lengths[index];
    descriptor->SectionOffset = sizeof(EFI_COMMON_ERROR_RECORD_HEADER)
        + (num_sections * sizeof(EFI_ERROR_SECTION_DESCRIPTOR));
    for (int i=0; i<index; i++)
        descriptor->SectionOffset += lengths[i];
    
    //Ensure the FRU text is not null terminated early.
    for (int i=0; i<20; i++)
    {
        if (descriptor->FruString[i] = 0x0)
            descriptor->FruString[i] = rand() % 127 + 1;

        //Null terminate last byte.
        if (i == 19)
            descriptor->FruString[i] = 0x0;
    }

    //Set section type GUID based on type name.
    if (strcmp(type, "generic") == 0)
        memcpy(&descriptor->SectionType, &gEfiProcessorGenericErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "ia32x64") == 0)
        memcpy(&descriptor->SectionType, &gEfiIa32X64ProcessorErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "ipf") == 0)
        memcpy(&descriptor->SectionType, &gEfiIpfProcessorErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "arm") == 0)
        memcpy(&descriptor->SectionType, &gEfiArmProcessorErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "memory") == 0)
        memcpy(&descriptor->SectionType, &gEfiPlatformMemoryErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "memory2") == 0)
        memcpy(&descriptor->SectionType, &gEfiPlatformMemoryError2SectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "pcie") == 0)
        memcpy(&descriptor->SectionType, &gEfiPcieErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "firmware") == 0)
        memcpy(&descriptor->SectionType, &gEfiFirmwareErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "pcibus") == 0)
        memcpy(&descriptor->SectionType, &gEfiPciBusErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "pcidev") == 0)
        memcpy(&descriptor->SectionType, &gEfiPciDevErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "dmargeneric") == 0)
        memcpy(&descriptor->SectionType, &gEfiDMArGenericErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "dmarvtd") == 0)
        memcpy(&descriptor->SectionType, &gEfiDirectedIoDMArErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "dmariommu") == 0)
        memcpy(&descriptor->SectionType, &gEfiIommuDMArErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "ccixper") == 0)
        memcpy(&descriptor->SectionType, &gEfiCcixPerLogErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "cxlprotocol") == 0)
        memcpy(&descriptor->SectionType, &gEfiCxlProtocolErrorSectionGuid, sizeof(EFI_GUID));
    else if (strcmp(type, "cxlcomponent") == 0)
    {
        //Choose between the different CXL component type GUIDs.
        int componentType = rand() % 5;
        switch (componentType)
        {
            case 0:
                memcpy(&descriptor->SectionType, &gEfiCxlGeneralMediaErrorSectionGuid, sizeof(EFI_GUID));
                break;
            case 1:
                memcpy(&descriptor->SectionType, &gEfiCxlDramEventErrorSectionGuid, sizeof(EFI_GUID));
                break;
            case 2:
                memcpy(&descriptor->SectionType, &gEfiCxlPhysicalSwitchErrorSectionGuid, sizeof(EFI_GUID));
                break;
            case 3:
                memcpy(&descriptor->SectionType, &gEfiCxlVirtualSwitchErrorSectionGuid, sizeof(EFI_GUID));
                break;
            default:
                memcpy(&descriptor->SectionType, &gEfiCxlMldPortErrorSectionGuid, sizeof(EFI_GUID));
                break;
        }
    }
    else if (strcmp(type, "unknown") != 0)
    {
        //Undefined section, show error.
        printf("Undefined section type '%s' provided. See 'cper-generate --help' for command information.\n", type);
        return 0;
    }

    return descriptor;
}

//Generates a single CPER section given the string type.
size_t generate_section(void** location, char* type)
{
    //The length of the section.
    size_t length = 0;

    //Switch on the type, generate accordingly.
    if (strcmp(type, "generic") == 0)
        length = generate_section_generic(location);
    else if (strcmp(type, "ia32x64") == 0)
        length = generate_section_ia32x64(location);
    // else if (strcmp(type, "ipf") == 0)
    //     length = generate_section_ipf(location);
    else if (strcmp(type, "arm") == 0)
        length = generate_section_arm(location);
    else if (strcmp(type, "memory") == 0)
        length = generate_section_memory(location);
    else if (strcmp(type, "memory2") == 0)
        length = generate_section_memory2(location);
    else if (strcmp(type, "pcie") == 0)
        length = generate_section_pcie(location);
    else if (strcmp(type, "firmware") == 0)
        length = generate_section_firmware(location);
    else if (strcmp(type, "pcibus") == 0)
        length = generate_section_pci_bus(location);
    else if (strcmp(type, "pcidev") == 0)
        length = generate_section_pci_dev(location);
    else if (strcmp(type, "dmargeneric") == 0)
        length = generate_section_dmar_generic(location);
    else if (strcmp(type, "dmarvtd") == 0)
        length = generate_section_dmar_vtd(location);
    else if (strcmp(type, "dmariommu") == 0)
        length = generate_section_dmar_iommu(location);
    else if (strcmp(type, "ccixper") == 0)
        length = generate_section_ccix_per(location);
    else if (strcmp(type, "cxlprotocol") == 0)
        length = generate_section_cxl_protocol(location);
    else if (strcmp(type, "cxlcomponent") == 0)
        length = generate_section_cxl_component(location);
    else if (strcmp(type, "unknown") == 0)
        length = generate_random_section(location, rand() % 256);
    else 
    {
        //Undefined section, show error.
        printf("Undefined section type '%s' given to generate. See 'cper-generate --help' for command information.\n", type);
        return 0;
    }

    return length;
}