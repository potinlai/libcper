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

//Tests a single randomly generated CPER section of the given type to ensure CPER-JSON IR validity.
void single_section_ir_test(const char *section_name)
{
	//Generate CPER record for the given type.
	char *buf;
	size_t size;
	FILE *record = generate_record_memstream(&section_name, 1, &buf, &size);

	//Convert to IR, free resources.
	json_object *ir = cper_to_ir(record);
	fclose(record);
	free(buf);

	//Validate against schema.
	char error_message[JSON_ERROR_MSG_MAX_LEN] = { 0 };
	int valid = validate_schema_from_file("./specification/cper-json.json",
					      ir, error_message);
	ASSERT_TRUE(valid) << error_message;
}

//Checks for binary round-trip equality for a given randomly generated CPER record.
void single_section_binary_test(const char *section_name)
{
	//Generate CPER record for the given type.
	char *buf;
	size_t size;
	FILE *record = generate_record_memstream(&section_name, 1, &buf, &size);

	//Convert to IR, then back to binary, getting a stream out.
	json_object *ir = cper_to_ir(record);
	char *cper_buf;
	size_t cper_buf_size;
	FILE *stream = open_memstream(&cper_buf, &cper_buf_size);
	ir_to_cper(ir, stream);
	size_t cper_len = ftell(stream);
	fclose(stream);

	//Validate the two are identical.
	ASSERT_GE(size, cper_len);
	ASSERT_EQ(memcmp(buf, cper_buf, cper_len), 0)
		<< "Binary output was not identical to input.";

	//Free everything up.
	fclose(record);
	free(buf);
	free(cper_buf);
}

/*
* Single section tests.
*/
//Generic processor tests.
TEST(GenericProcessorTests, IRValid)
{
	single_section_ir_test("generic");
}
TEST(GenericProcessorTests, BinaryEqual)
{
	single_section_binary_test("generic");
}

//IA32/x64 tests.
TEST(IA32x64Tests, IRValid)
{
	single_section_ir_test("ia32x64");
}
TEST(IA32x64Tests, BinaryEqual)
{
	single_section_binary_test("ia32x64");
}

// TEST(IPFTests, IRValid) {
//     single_section_ir_test("ipf");
// }

//ARM tests.
TEST(ArmTests, IRValid)
{
	single_section_ir_test("arm");
}
TEST(ArmTests, BinaryEqual)
{
	single_section_binary_test("arm");
}

//Memory tests.
TEST(MemoryTests, IRValid)
{
	single_section_ir_test("memory");
}
TEST(MemoryTests, BinaryEqual)
{
	single_section_binary_test("memory");
}

//Memory 2 tests.
TEST(Memory2Tests, IRValid)
{
	single_section_ir_test("memory2");
}
TEST(Memory2Tests, BinaryEqual)
{
	single_section_binary_test("memory2");
}

//PCIe tests.
TEST(PCIeTests, IRValid)
{
	single_section_ir_test("pcie");
}
TEST(PCIeTests, BinaryEqual)
{
	single_section_binary_test("pcie");
}

//Firmware tests.
TEST(FirmwareTests, IRValid)
{
	single_section_ir_test("firmware");
}
TEST(FirmwareTests, BinaryEqual)
{
	single_section_binary_test("firmware");
}

//PCI Bus tests.
TEST(PCIBusTests, IRValid)
{
	single_section_ir_test("pcibus");
}
TEST(PCIBusTests, BinaryEqual)
{
	single_section_binary_test("pcibus");
}

//PCI Device tests.
TEST(PCIDevTests, IRValid)
{
	single_section_ir_test("pcidev");
}
TEST(PCIDevTests, BinaryEqual)
{
	single_section_binary_test("pcidev");
}

//Generic DMAr tests.
TEST(DMArGenericTests, IRValid)
{
	single_section_ir_test("dmargeneric");
}
TEST(DMArGenericTests, BinaryEqual)
{
	single_section_binary_test("dmargeneric");
}

//VT-d DMAr tests.
TEST(DMArVtdTests, IRValid)
{
	single_section_ir_test("dmarvtd");
}
TEST(DMArVtdTests, BinaryEqual)
{
	single_section_binary_test("dmarvtd");
}

//IOMMU DMAr tests.
TEST(DMArIOMMUTests, IRValid)
{
	single_section_ir_test("dmariommu");
}
TEST(DMArIOMMUTests, BinaryEqual)
{
	single_section_binary_test("dmariommu");
}

//CCIX PER tests.
TEST(CCIXPERTests, IRValid)
{
	single_section_ir_test("ccixper");
}
TEST(CCIXPERTests, BinaryEqual)
{
	single_section_binary_test("ccixper");
}

//CXL Protocol tests.
TEST(CXLProtocolTests, IRValid)
{
	single_section_ir_test("cxlprotocol");
}
TEST(CXLProtocolTests, BinaryEqual)
{
	single_section_binary_test("cxlprotocol");
}

//CXL Component tests.
TEST(CXLComponentTests, IRValid)
{
	single_section_ir_test("cxlcomponent");
}
TEST(CXLComponentTests, BinaryEqual)
{
	single_section_binary_test("cxlcomponent");
}

//Unknown section tests.
TEST(UnknownSectionTests, IRValid)
{
	single_section_ir_test("unknown");
}
TEST(UnknownSectionTests, BinaryEqual)
{
	single_section_binary_test("unknown");
}

//Entrypoint for the testing program.
int main()
{
	testing::InitGoogleTest();
	return RUN_ALL_TESTS();
}