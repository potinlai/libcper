#include <stdio.h>
#include "../cper-parse.h"
#include "json.h"

int main(int argc, char* argv[]) {

    //Get a handle for the log file.
    FILE* cper_file = fopen(argv[1], "r");
    if (cper_file == NULL) {
        printf("Could not open CPER record, file handle returned null.");
        return -1;
    }

    json_object* ir = cper_to_ir(cper_file);
    fclose(cper_file);

    printf("\n%s\n", json_object_to_json_string(ir));
}