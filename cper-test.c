#include <stdio.h>
#include "cper-parse.h"
#include "json.h"

int main(int argc, char* argv[]) {
    json_object* ir = cper_to_ir(argv[1]);
    printf("\n%s\n", json_object_to_json_string(ir));
}