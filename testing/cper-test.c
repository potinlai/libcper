#include <stdio.h>
#include "../cper-parse.h"
#include "json.h"
#include "../json-schema.h"

int main(int argc, char* argv[]) {

    //Get a handle for the log file.
    FILE* cper_file = fopen(argv[1], "r");
    if (cper_file == NULL) {
        printf("Could not open CPER record, file handle returned null.");
        return -1;
    }

    json_object* ir = cper_to_ir(cper_file);
    fclose(cper_file);

    const char* json_output = json_object_to_json_string(ir);
    printf("\n%s\n", json_output);

    //Test JSON validator.
    if (argc >= 3)
    {
        printf("Validating output with specification %s...\n", argv[2]);
        char* error_message = malloc(300);
        if (!validate_schema_from_file(argv[2], ir, error_message))
        {
            printf("Validation failed: %s\n", error_message);
        }
        else
        {
            printf("Validation passed!\n");
        }
    }
}