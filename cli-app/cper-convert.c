/**
 * A user-space application linking to the CPER-JSON conversion library which allows for easy
 * conversion between CPER and CPER-JSON formats. 
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <limits.h>
#include "json.h"
#include "../cper-parse.h"
#include "../json-schema.h"

void cper_to_json(int argc, char *argv[]);
void json_to_cper(int argc, char *argv[]);
void print_help(void);

int main(int argc, char *argv[])
{
	//Ensure at least one argument is present.
	if (argc < 2) {
		printf("Invalid number of arguments. See 'cper-convert --help' for command information.\n");
		return -1;
	}

	//Parse the command line arguments.
	char* input_file = NULL;
	char* output_file = NULL;

	//Run the requested command.
	if (strcmp(argv[1], "to-json") == 0)
		cper_to_json(argc, argv);
	else if (strcmp(argv[1], "to-cper") == 0)
		json_to_cper(argc, argv);
	else if (strcmp(argv[1], "--help") == 0)
		print_help();
	else {
		printf("Unrecognised argument '%s'. See 'cper-convert --help' for command information.\n",
		       argv[1]);
		return -1;
	}

	return 0;
}

//Command for converting a provided CPER log file into JSON.
void cper_to_json(int argc, char *argv[])
{
	if (argc < 3) {
		printf("Insufficient number of arguments for 'to-json'. See 'cper-convert --help' for command information.\n");
		return;
	}

	//Get a handle for the log file.
	FILE *cper_file = fopen(argv[2], "r");
	if (cper_file == NULL) {
		printf("Could not open provided CPER file '%s', file handle returned null.\n",
		       argv[2]);
		return;
	}

	//Convert.
	json_object *ir = cper_to_ir(cper_file);
	fclose(cper_file);
	const char *json_output =
		json_object_to_json_string_ext(ir, JSON_C_TO_STRING_PRETTY);

	//Check whether there is a "--out" argument, if there is, then output to file instead.
	//Otherwise, just send to console.
	if (argc != 5) {
		printf("%s\n", json_output);
		return;
	}

	//File out argument exists. Argument valid?
	if (strcmp(argv[3], "--out") != 0) {
		printf("Invalid argument '%s' for command 'to-json'. See 'cper-convert --help' for command information.\n",
		       argv[3]);
		return;
	}

	//Try to open a file handle to the desired output file.
	FILE *json_file = fopen(argv[4], "w");
	if (json_file == NULL) {
		printf("Could not get a handle for output file '%s', file handle returned null.\n",
		       argv[4]);
		return;
	}

	//Write out to file.
	fwrite(json_output, strlen(json_output), 1, json_file);
	fclose(json_file);
}

//Command for converting a provided CPER-JSON JSON file to CPER binary.
void json_to_cper(int argc, char *argv[])
{
	if (argc < 5) {
		printf("Insufficient number of arguments for 'to-cper'. See 'cper-convert --help' for command information.\n");
		return;
	}

	//Read JSON IR from file.
	json_object *ir = json_object_from_file(argv[2]);
	if (ir == NULL) {
		printf("Could not read JSON from file '%s', import returned null.\n",
		       argv[2]);
		return;
	}

	//Are we skipping validation?
	int do_validate = 1;
	if (argc >= 6 && argc < 8) {
		if (strcmp(argv[5], "--no-validate") == 0) {
			do_validate = 0;
		}
	}

	//Validate the JSON against specification, unless otherwise specified.
	if (do_validate) {
		char *specification_path = NULL;

		//Is there a specification file path?
		if (argc >= 7 &&
		    strcmp(argv[argc - 2], "--specification") == 0) {
			specification_path = argv[argc - 1];
		} else {
			//Make the specification path the assumed default (application directory + specification/cper-json.json).
			specification_path = malloc(PATH_MAX);
			char *dir = dirname(argv[0]);
			strcpy(specification_path, dir);
			strcat(specification_path,
			       "/specification/cper-json.json");
		}

		//Enable debug mode if indicated.
		for (int i = 5; i < argc; i++) {
			if (strcmp(argv[i], "--debug") == 0) {
				validate_schema_debug_enable();
				printf("debug enabled.\n");
				break;
			}
		}

		//Attempt to verify with the the specification.
		char *error_message = malloc(JSON_ERROR_MSG_MAX_LEN);
		int success = validate_schema_from_file(specification_path, ir,
							error_message);

		//Free specification path (if necessary).
		if (argc == 5)
			free(specification_path);

		//If failed, early exit before conversion.
		if (!success) {
			printf("JSON format validation failed: %s\n",
			       error_message);
			free(error_message);
			return;
		}
		free(error_message);
	}

	//Attempt a conversion.
	//Open a read for the output file.
	FILE *cper_file = fopen(argv[4], "w");
	if (cper_file == NULL) {
		printf("Could not open output file '%s', file handle returned null.\n",
		       argv[4]);
		return;
	}

	//Run the converter.
	ir_to_cper(ir, cper_file);
	fclose(cper_file);
}

//Command for printing help information.
void print_help(void)
{
	printf(":: to-json cper.file [--out file.name]\n");
	printf("\tConverts the provided CPER log file into JSON, by default outputting to console. If '--out' is specified,\n");
	printf("\tThe outputted JSON will be written to the provided file name instead.\n");
	printf("\n:: to-cper cper.json --out file.name [--no-validate] [--debug] [--specification some/spec/path.json]\n");
	printf("\tConverts the provided CPER-JSON JSON file into CPER binary. An output file must be specified with '--out'.\n");
	printf("\tBy default, the provided JSON will try to be validated against a specification. If no specification file path\n");
	printf("\tis provided with '--specification', then it will default to 'argv[0] + /specification/cper-json.json'.\n");
	printf("\tIf the '--no-validate' argument is set, then the provided JSON will not be validated. Be warned, this may cause\n");
	printf("\tpremature exit/unexpected behaviour in CPER output.\n");
	printf("\n:: --help\n");
	printf("\tDisplays help information to the console.\n");
}