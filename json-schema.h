#ifndef JSON_SCHEMA_H
#define JSON_SCHEMA_H

#include "json.h"

#define JSON_SCHEMA_VERSION "https://json-schema.org/draft/2020-12/schema"

int validate_schema(json_object* schema, char* schema_directory, json_object* object, char* error_message);
int validate_schema_from_file(const char* schema_file, json_object* object, char* error_message);

#endif