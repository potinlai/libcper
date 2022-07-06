/**
 * Describes high level functions for converting an entire CPER log, and functions for parsing 
 * CPER headers and section descriptions into an intermediate JSON format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "edk/Cper.h"
#include "cper-parse.h"
#include "cper-utils.h"
#include "sections/cper-section-generic.h"
#include "sections/cper-section-ia32x64.h"
#include "sections/cper-section-arm.h"
#include "sections/cper-section-memory.h"
#include "sections/cper-section-pcie.h"
#include "sections/cper-section-pci-bus.h"
#include "sections/cper-section-pci-dev.h"

//Private pre-definitions.
json_object* cper_header_to_ir(EFI_COMMON_ERROR_RECORD_HEADER* header);
json_object* cper_section_descriptor_to_ir(EFI_ERROR_SECTION_DESCRIPTOR* section_descriptor);
json_object* cper_section_to_ir(FILE* handle, EFI_ERROR_SECTION_DESCRIPTOR* descriptor);

//Reads a CPER log file at the given file location, and returns an intermediate
//JSON representation of this CPER record.
json_object* cper_to_ir(const char* filename) 
{
    //Get a handle for the log file.
    FILE* cper_file = fopen(filename, "r");
    if (cper_file == NULL) {
        printf("Could not open CPER record, file handle returned null.");
        return NULL;
    }

    //Ensure this is really a CPER log.
    EFI_COMMON_ERROR_RECORD_HEADER header;
    fseek(cper_file, 0, SEEK_SET);
    if (fread(&header, sizeof(EFI_COMMON_ERROR_RECORD_HEADER), 1, cper_file) != 1)
    {
        printf("Invalid CPER file: Invalid length (log too short).");
        return NULL;
    }

    //Check if the header contains the magic bytes ("CPER").
    if (header.SignatureStart != EFI_ERROR_RECORD_SIGNATURE_START) {
        printf("Invalid CPER file: Invalid header (incorrect signature).");
        return NULL;
    }

    // //Print struct contents (debug).
    // fpos_t file_pos;
    // fgetpos(cper_file, &file_pos);
    // printf("Stream is at position %d.\n", file_pos);
    // printf("SignatureStart: %s\n", (char*)&header.SignatureStart);
    // printf("Revision: %u\n", header.Revision);
    // printf("SectionCount: %u\n", header.SectionCount);
    // printf("Severity: %d\n", header.ErrorSeverity);
    // printf("RecordLength: %d\n", header.RecordLength);

    //Create the header JSON object from the read bytes.
    json_object* header_ir = cper_header_to_ir(&header);

    //Read the appropriate number of section descriptors & sections, and convert them into IR format.
    json_object* section_descriptors_ir = json_object_new_array();
    json_object* sections_ir = json_object_new_array();
    for (int i=0; i<header.SectionCount; i++)
    {
        //Create the section descriptor.
        EFI_ERROR_SECTION_DESCRIPTOR section_descriptor;
        if (fread(&section_descriptor, sizeof(EFI_ERROR_SECTION_DESCRIPTOR), 1, cper_file) != 1)
        {
            printf("Invalid number of section headers: Header states %d sections, could not read section %d.", header.SectionCount, i+1);
            return NULL;
        }
        json_object_array_add(section_descriptors_ir, cper_section_descriptor_to_ir(&section_descriptor));

        //Read the section itself.
        json_object_array_add(sections_ir, cper_section_to_ir(cper_file, &section_descriptor));
    }

    //Add the header, section descriptors, and sections to a parent object.
    json_object* parent = json_object_new_object();
    json_object_object_add(parent, "header", header_ir);
    json_object_object_add(parent, "sectionDescriptors", section_descriptors_ir);
    json_object_object_add(parent, "sections", sections_ir);

    //...
    return parent;
}

//Converts a parsed CPER record header into intermediate JSON object format.
json_object* cper_header_to_ir(EFI_COMMON_ERROR_RECORD_HEADER* header) 
{
    json_object* header_ir = json_object_new_object();

    //Revision/version information.
    json_object_object_add(header_ir, "revision", revision_to_ir(header->Revision));

    //Section count.
    json_object_object_add(header_ir, "sectionCount", json_object_new_int(header->SectionCount));

    //Error severity (with interpreted string version).
    json_object* error_severity = json_object_new_object();
    json_object_object_add(error_severity, "code", json_object_new_int(header->ErrorSeverity));
    json_object_object_add(error_severity, "name", json_object_new_string(severity_to_string(header->ErrorSeverity)));
    json_object_object_add(header_ir, "severity", error_severity);

    //The validation bits for each section.
    json_object* validation_bits = bitfield_to_ir(header->ValidationBits, 3, CPER_HEADER_VALID_BITFIELD_NAMES);
    json_object_object_add(header_ir, "validationBits", validation_bits);

    //Total length of the record (including headers) in bytes.
    json_object_object_add(header_ir, "recordLength", json_object_new_int(header->RecordLength));

    //If a timestamp exists according to validation bits, then add it.
    if (header->ValidationBits & 0b10)
    {
        char timestamp_string[TIMESTAMP_LENGTH];
        sprintf(timestamp_string, "%02d%02d-%02d-%02dT%02d:%02d:%02d.000", 
            header->TimeStamp.Century,
            header->TimeStamp.Year,
            header->TimeStamp.Month,
            header->TimeStamp.Day,
            header->TimeStamp.Hours,
            header->TimeStamp.Minutes,
            header->TimeStamp.Seconds);

        json_object_object_add(header_ir, "timestamp", json_object_new_string(timestamp_string));
        json_object_object_add(header_ir, "timestampIsPrecise", json_object_new_boolean(header->TimeStamp.Flag));
    }

    //If a platform ID exists according to the validation bits, then add it.
    if (header->ValidationBits & 0b1) 
    {
        char platform_string[GUID_STRING_LENGTH];
        guid_to_string(platform_string, &header->PlatformID);
        json_object_object_add(header_ir, "platformID", json_object_new_string(platform_string));
    }

    //If a partition ID exists according to the validation bits, then add it.
    if (header->ValidationBits & 0b100)
    {
        char partition_string[GUID_STRING_LENGTH];
        guid_to_string(partition_string, &header->PartitionID);
        json_object_object_add(header_ir, "partitionID", json_object_new_string(partition_string));
    }

    //Creator ID of the header.
    char creator_string[GUID_STRING_LENGTH];
    guid_to_string(creator_string, &header->CreatorID);
    json_object_object_add(header_ir, "creatorID", json_object_new_string(creator_string));

    //Notification type for the header. Some defined types are available.
    json_object* notification_type = json_object_new_object();
    char notification_type_string[GUID_STRING_LENGTH];
    guid_to_string(notification_type_string, &header->NotificationType);
    json_object_object_add(notification_type, "guid", json_object_new_string(notification_type_string));
    
    //Add the human readable notification type if possible.
    char* notification_type_readable = "Unknown";
    if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeCmcGuid))
        notification_type_readable = "CMC";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeCpeGuid))
        notification_type_readable = "CPE";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeMceGuid))
        notification_type_readable = "MCE";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypePcieGuid))
        notification_type_readable = "PCIe";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeInitGuid))
        notification_type_readable = "INIT";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeNmiGuid))
        notification_type_readable = "NMI";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeBootGuid))
        notification_type_readable = "Boot";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeDmarGuid))
        notification_type_readable = "DMAr";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeSeaGuid))
        notification_type_readable = "SEA";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeSeiGuid))
        notification_type_readable = "SEI";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypePeiGuid))
        notification_type_readable = "PEI";
    else if (guid_equal(&header->NotificationType, &gEfiEventNotificationTypeCxlGuid))
        notification_type_readable = "CXL Component";
    json_object_object_add(notification_type, "type", json_object_new_string(notification_type_readable));
    json_object_object_add(header_ir, "notificationType", notification_type);

    //The record ID for this record, unique on a given system.
    json_object_object_add(header_ir, "recordID", json_object_new_uint64(header->RecordID));

    //Flag for the record, and a human readable form.
    json_object* flags = integer_to_readable_pair(header->Flags,
        sizeof(CPER_HEADER_FLAG_TYPES_KEYS) / sizeof(int),
        CPER_HEADER_FLAG_TYPES_KEYS,
        CPER_HEADER_FLAG_TYPES_VALUES,
        "Unknown");
    json_object_object_add(header_ir, "flags", flags);

    //Persistence information. Outside the scope of specification, so just a uint32 here.
    json_object_object_add(header_ir, "persistenceInformation", json_object_new_uint64(header->PersistenceInfo));
    return header_ir;
}

//Converts the given EFI section descriptor into JSON IR format.
json_object* cper_section_descriptor_to_ir(EFI_ERROR_SECTION_DESCRIPTOR* section_descriptor)
{
    json_object* section_descriptor_ir = json_object_new_object();

    //The offset of the section from the base of the record header, length.
    json_object_object_add(section_descriptor_ir, "sectionOffset", json_object_new_int(section_descriptor->SectionOffset));
    json_object_object_add(section_descriptor_ir, "sectionLength", json_object_new_int(section_descriptor->SectionLength));

    //Revision.
    json_object_object_add(section_descriptor_ir, "revision", revision_to_ir(section_descriptor->Revision));

    //Validation bits.
    json_object* validation_bits = json_object_new_object();
    json_object_object_add(validation_bits, "fruID", json_object_new_boolean(section_descriptor->SecValidMask & 0b1));
    json_object_object_add(validation_bits, "fruString", json_object_new_boolean((section_descriptor->SecValidMask & 0b10) >> 1));
    json_object_object_add(section_descriptor_ir, "validationBits", validation_bits);

    //Flag bits.
    json_object* flags = bitfield_to_ir(section_descriptor->SectionFlags, 8, CPER_SECTION_DESCRIPTOR_FLAGS_BITFIELD_NAMES);
    json_object_object_add(section_descriptor_ir, "flags", flags);

    //Section type (GUID).
    json_object* section_type = json_object_new_object();
    char section_type_string[GUID_STRING_LENGTH];
    guid_to_string(section_type_string, &section_descriptor->SectionType);
    json_object_object_add(section_type, "data", json_object_new_string(section_type_string));

    //Readable section type, if possible.
    char* section_type_readable = "Unknown";
    if (guid_equal(&section_descriptor->SectionType, &gEfiProcessorGenericErrorSectionGuid))
        section_type_readable = "Processor Generic";
    if (guid_equal(&section_descriptor->SectionType, &gEfiIa32X64ProcessorErrorSectionGuid))
        section_type_readable = "IA32/X64";
    //todo: Why does IPF have an overly long GUID?
    // if (guid_equal(&section_descriptor->SectionType, &gEfiIpfProcessorErrorSectionGuid))
    //     section_type_readable = "IPF";
    if (guid_equal(&section_descriptor->SectionType, &gEfiArmProcessorErrorSectionGuid))
        section_type_readable = "ARM";
    if (guid_equal(&section_descriptor->SectionType, &gEfiPlatformMemoryErrorSectionGuid))
        section_type_readable = "Platform Memory";
    if (guid_equal(&section_descriptor->SectionType, &gEfiPcieErrorSectionGuid))
        section_type_readable = "PCIe";
    if (guid_equal(&section_descriptor->SectionType, &gEfiFirmwareErrorSectionGuid))
        section_type_readable = "Firmware Error Record Reference";
    if (guid_equal(&section_descriptor->SectionType, &gEfiPciBusErrorSectionGuid))
        section_type_readable = "PCI/PCI-X Bus";
    if (guid_equal(&section_descriptor->SectionType, &gEfiPciDevErrorSectionGuid))
        section_type_readable = "PCI Component/Device";
    if (guid_equal(&section_descriptor->SectionType, &gEfiDMArGenericErrorSectionGuid))
        section_type_readable = "DMAr Generic";
    if (guid_equal(&section_descriptor->SectionType, &gEfiDirectedIoDMArErrorSectionGuid))
        section_type_readable = "Intel VT for Directed I/O specific DMAr section";
    if (guid_equal(&section_descriptor->SectionType, &gEfiIommuDMArErrorSectionGuid))
        section_type_readable = "IOMMU specific DMAr section";

    //todo: How do you determine if this is a CXL component event?
    // if (guid_equal(&section_descriptor->SectionType, &gEfiProcessorGenericErrorSectionGuid))
    //     section_type_readable = "CXL Component Event";

    json_object_object_add(section_type, "type", json_object_new_string(section_type_readable));
    json_object_object_add(section_descriptor_ir, "sectionType", section_type);

    //If validation bits indicate it exists, add FRU ID.
    if (section_descriptor->SecValidMask & 0b1)
    {
        char fru_id_string[GUID_STRING_LENGTH];
        guid_to_string(fru_id_string, &section_descriptor->FruId);
        json_object_object_add(section_descriptor_ir, "fruID", json_object_new_string(fru_id_string));
    }

    //If validation bits indicate it exists, add FRU text.
    if ((section_descriptor->SecValidMask & 0b10) >> 1)
        json_object_object_add(section_descriptor_ir, "fruText", json_object_new_string(section_descriptor->FruString));

    //Section severity.
    json_object* section_severity = json_object_new_object();
    json_object_object_add(section_severity, "code", json_object_new_int(section_descriptor->Severity));
    json_object_object_add(section_severity, "name", json_object_new_string(severity_to_string(section_descriptor->Severity)));
    json_object_object_add(section_descriptor_ir, "severity", section_severity);

    return section_descriptor_ir;
}


//Converts the section described by a single given section descriptor.
json_object* cper_section_to_ir(FILE* handle, EFI_ERROR_SECTION_DESCRIPTOR* descriptor)
{
    //Read section as described by the section descriptor.
    fseek(handle, descriptor->SectionOffset, SEEK_SET);
    void* section = calloc(1, descriptor->SectionLength);
    if (fread(section, descriptor->SectionLength, 1, handle) != 1)
    {
        printf("Section read failed: Could not read %d bytes from global offset %d.", 
            descriptor->SectionLength,
            descriptor->SectionOffset);
    }

    json_object* result = NULL;
    if (guid_equal(&descriptor->SectionType, &gEfiProcessorGenericErrorSectionGuid))
        result = cper_section_generic_to_ir(section, descriptor);
    else if (guid_equal(&descriptor->SectionType, &gEfiIa32X64ProcessorErrorSectionGuid))
        result = cper_section_ia32x64_to_ir(section, descriptor);
    // //todo: Why does IPF have an overly long GUID?
    // // if (guid_equal(&descriptor->SectionType, &gEfiIpfProcessorErrorSectionGuid))
    else if (guid_equal(&descriptor->SectionType, &gEfiArmProcessorErrorSectionGuid))
        result = cper_section_arm_to_ir(section, descriptor);
    else if (guid_equal(&descriptor->SectionType, &gEfiPlatformMemoryErrorSectionGuid))
        result = cper_section_platform_memory_to_ir(section, descriptor);
    else if (guid_equal(&descriptor->SectionType, &gEfiPlatformMemoryError2SectionGuid))
        result = cper_section_platform_memory2_to_ir(section, descriptor);
    else if (guid_equal(&descriptor->SectionType, &gEfiPcieErrorSectionGuid))
        result = cper_section_pcie_to_ir(section, descriptor);
    // if (guid_equal(&descriptor->SectionType, &gEfiFirmwareErrorSectionGuid))
    //     result = cper_section_firmware_error_to_ir(section);
    else if (guid_equal(&descriptor->SectionType, &gEfiPciBusErrorSectionGuid))
        result = cper_section_pci_bus_to_ir(section, descriptor);
    else if (guid_equal(&descriptor->SectionType, &gEfiPciDevErrorSectionGuid))
        result = cper_section_pci_dev_to_ir(section, descriptor);
    // if (guid_equal(&descriptor->SectionType, &gEfiDMArGenericErrorSectionGuid))
    //     result = cper_section_dmar_generic_to_ir(section);
    // if (guid_equal(&descriptor->SectionType, &gEfiDirectedIoDMArErrorSectionGuid))
    //     result = cper_section_intel_io_dma_to_ir(section);
    // if (guid_equal(&descriptor->SectionType, &gEfiIommuDMArErrorSectionGuid))
    //     result = cper_section_iommu_dma_to_ir(section);

    //Free section memory, return result.
    free(section);
    return result;
}