#include <stdio.h>
#include <string.h>
#include "../cper-parse.h"
#include "json.h"
#include "../json-schema.h"

void test_ir_to_cper(int argc, char* argv[]);
void test_cper_to_ir(int argc, char* argv[]);

int main(int argc, char* argv[]) 
{
    if (!strcmp(argv[1], "convert-to-json"))
        test_cper_to_ir(argc, argv);
    else if (!strcmp(argv[1], "convert-to-cper"))
        test_ir_to_cper(argc, argv);
    else
    {
        printf("Invalid command provided. Must be one of 'convert-to-json' or 'convert-to-cper'.\n");    
    }

    return 0;    
}

void test_ir_to_cper(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("Insufficient number of arguments.\n");
        return;
    }

    //Read JSON IR from file.
    json_object* ir = json_object_from_file(argv[2]);
    if (ir == NULL)
    {
        printf("Could not read IR JSON, import returned null.");
        return;
    }

    //Open a read for the output file.
    FILE* cper_file = fopen(argv[3], "w");
    if (cper_file == NULL) {
        printf("Could not open output file, file handle returned null.");
        return;
    }

    //Run the converter.
    ir_to_cper(ir, cper_file);
    fclose(cper_file);
    printf("Conversion finished!\n");
}

void test_cper_to_ir(int argc, char* argv[])
{
    //Get a handle for the log file.
    FILE* cper_file = fopen(argv[2], "r");
    if (cper_file == NULL) {
        printf("Could not open CPER record, file handle returned null.");
        return;
    }

    json_object* ir = cper_to_ir(cper_file);
    fclose(cper_file);

    const char* json_output = json_object_to_json_string(ir);
    printf("\n%s\n", json_output);

    //Test JSON validator.
    if (argc >= 4)
    {
        printf("Validating output with specification %s...\n", argv[3]);
        validate_schema_debug_enable();
        char* error_message = malloc(JSON_ERROR_MSG_MAX_LEN);
        if (!validate_schema_from_file(argv[3], ir, error_message))
        {
            printf("Validation failed: %s\n", error_message);
        }
        else
        {
            printf("Validation passed!\n");
        }
    }
}