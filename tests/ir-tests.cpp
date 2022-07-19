/**
 * Defines tests for validating CPER-JSON IR output from the cper-parse library.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include "gtest/gtest.h"
#include "test-utils.hpp"
extern "C" {
#include "json.h"
#include "../cper-parse.h"
#include "../json-schema.h"
#include "../generator/cper-generate.h"
}

/*
* Test templates.
*/
void single_section_ir_test(const char* section_name)
{
    //Generate CPER record for generic processor.
    char* buf;
    size_t size;
    FILE* record = generate_record_memstream(&section_name, 1, &buf, &size);

    //Convert to IR, free resources.
    json_object* ir = cper_to_ir(record);
    fclose(record);
    free(buf);

    //Validate against schema.
    char error_message[JSON_ERROR_MSG_MAX_LEN] = {0};
    int valid = validate_schema_from_file("./specification/cper-json.json", ir, error_message);
    ASSERT_TRUE(valid) << error_message;
}

/*
* Single section tests.
*/
TEST(GenericProcessorTests, IRValid) {
    single_section_ir_test("generic");
}
TEST(IA32x64Tests, IRValid) {
    single_section_ir_test("ia32x64");
}
// TEST(IPFTests, IRValid) {
//     single_section_ir_test("ipf");
// }
TEST(ArmTests, IRValid) {
    single_section_ir_test("arm");
}
TEST(MemoryTests, IRValid) {
    single_section_ir_test("memory");
}
TEST(Memory2Tests, IRValid) {
    single_section_ir_test("memory2");
}
TEST(PCIeTests, IRValid) {
    single_section_ir_test("pcie");
}
TEST(FirmwareTests, IRValid) {
    single_section_ir_test("firmware");
}
TEST(PCIBusTests, IRValid) {
    single_section_ir_test("pcibus");
}
TEST(PCIDevTests, IRValid) {
    single_section_ir_test("pcidev");
}
TEST(DMArGenericTests, IRValid) {
    single_section_ir_test("dmargeneric");
}
TEST(DMArVtdTests, IRValid) {
    single_section_ir_test("dmarvtd");
}
TEST(DMArIOMMUTests, IRValid) {
    single_section_ir_test("dmariommu");
}
TEST(CCIXPERTests, IRValid) {
    single_section_ir_test("ccixper");
}
TEST(CXLProtocolTests, IRValid) {
    single_section_ir_test("cxlprotocol");
}
TEST(CXLComponentTests, IRValid) {
    single_section_ir_test("cxlcomponent");
}
TEST(UnknownSectionTests, IRValid) {
    single_section_ir_test("unknown");
}

//Entrypoint for the testing program.
int main()
{
    testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}