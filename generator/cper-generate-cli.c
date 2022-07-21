/**
 * A user-space application for generating psuedo-random specification compliant CPER records. 
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include <string.h>
#include "../edk/Cper.h"
#include "cper-generate.h"

void print_help();

int main(int argc, char *argv[])
{
	//If help requested, print help.
	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		print_help();
		return 0;
	}

	//Ensure the minimum number of arguments.
	if (argc < 5) {
		printf("Insufficient number of arguments. See 'cper-generate --help' for command information.\n");
		return -1;
	}

	//Open a file handle to write output.
	FILE *cper_file = fopen(argv[2], "w");
	if (cper_file == NULL) {
		printf("Could not get a handle for output file '%s', file handle returned null.\n",
		       argv[2]);
		return -1;
	}

	//Generate the record. Type names start from argv[4].
	UINT16 num_sections = argc - 4;
	generate_cper_record(argv + 4, num_sections, cper_file);
	fclose(cper_file);
}

//Prints command help for this CPER generator.
void print_help()
{
	printf(":: --out cper.file --sections section1 [section2 section3 ...]\n");
	printf("\tGenerates a psuedo-random CPER file with the provided section types and outputs to the given file name.\n");
	printf("\tValid section type names are the following:\n");
	printf("\t\t- generic\n");
	printf("\t\t- ia32x64\n");
	printf("\t\t- ipf\n");
	printf("\t\t- arm\n");
	printf("\t\t- memory\n");
	printf("\t\t- memory2\n");
	printf("\t\t- pcie\n");
	printf("\t\t- firmware\n");
	printf("\t\t- pcibus\n");
	printf("\t\t- pcidev\n");
	printf("\t\t- dmargeneric\n");
	printf("\t\t- dmarvtd\n");
	printf("\t\t- dmariommu\n");
	printf("\t\t- ccixper\n");
	printf("\t\t- cxlprotocol\n");
	printf("\t\t- cxlcomponent\n");
	printf("\t\t- unknown\n");
	printf("\n:: --help\n");
	printf("\tDisplays help information to the console.\n");
}