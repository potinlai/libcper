/**
 * A user-space application for generating pseudo-random specification compliant CPER records. 
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include <stdlib.h>
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

	//Parse the command line arguments.
	char *out_file = NULL;
	char **sections = NULL;
	UINT16 num_sections = 0;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--out") == 0 && i < argc - 1) {
			out_file = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "--sections") == 0 && i < argc - 1) {
			//All arguments after this must be section names.
			num_sections = argc - i - 1;
			sections = malloc(sizeof(char *) * num_sections);
			i++;
			
			for (int j = i; j < argc; j++)
				sections[j - i] = argv[j];
			break;
		} else {
			printf("Unrecognised argument '%s'. For command information, refer to 'cper-generate --help'.\n",
			       argv[i]);
			return -1;
		}
	}

	//If no output file passed as argument, exit.
	if (out_file == NULL) {
		printf("No output file provided. For command information, refer to 'cper-generate --help'.\n");
		return -1;
	}

	//Open a file handle to write output.
	FILE *cper_file = fopen(out_file, "w");
	if (cper_file == NULL) {
		printf("Could not get a handle for output file '%s', file handle returned null.\n",
		       out_file);
		return -1;
	}

	//Generate the record. Type names start from argv[4].
	generate_cper_record(sections, num_sections, cper_file);

	//Close & free remaining resources.
	fclose(cper_file);
	if (sections != NULL)
		free(sections);
}

//Prints command help for this CPER generator.
void print_help()
{
	printf(":: --out cper.file --sections section1 [section2 section3 ...]\n");
	printf("\tGenerates a pseudo-random CPER file with the provided section types and outputs to the given file name.\n");
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