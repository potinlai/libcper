/**
 * A very basic, non-complete implementation of a validator for the JSON Schema specification,
 * for validating CPER-JSON.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include "json.h"
#include "json-schema.h"
#include "edk/BaseTypes.h"

//Private pre-definitions.
int validate_field(const char* name, json_object* schema, json_object* object, char* error_message);
int validate_integer(const char* field_name, json_object* schema, json_object* object, char* error_message);
int validate_string(const char* field_name, json_object* schema, json_object* object, char* error_message);
int validate_object(const char* field_name, json_object* schema, json_object* object, char* error_message);
int validate_array(const char* field_name, json_object* schema, json_object* object, char* error_message);

//Validates a single JSON object against a provided schema file, returning 1 on success and 0 on failure to validate.
//Error message space must be allocated prior to call.
int validate_schema_from_file(const char* schema_file, json_object* object, char* error_message)
{
    //Load schema IR from file.
    json_object* schema_ir = json_object_from_file(schema_file);
    if (schema_ir == NULL)
    {
        sprintf(error_message, "Failed to load schema from file '%s'.", schema_file);
        return 0;
    }

    //Get the directory of the file.
    char* schema_file_copy = malloc(strlen(schema_file) + 1);
    strcpy(schema_file_copy, schema_file);
    char* schema_dir = dirname(schema_file_copy);

    int result = validate_schema(schema_ir, schema_dir, object, error_message);

    //Free memory from directory call.
    free(schema_file_copy);

    return result;
}

//Validates a single JSON object against a provided schema, returning 1 on success and 0 on failure to validate.
//Error message space must be allocated prior to call.
//If the schema does not include any other sub-schemas using "$ref", then leaving schema_directory as NULL is valid.
int validate_schema(json_object* schema, char* schema_directory, json_object* object, char* error_message)
{
    //Check that the schema version is the same as this validator.
    json_object* schema_ver = json_object_object_get(schema, "$schema");
    if (schema_ver == NULL || strcmp(json_object_get_string(schema_ver), JSON_SCHEMA_VERSION))
    {
        sprintf(error_message, "Provided schema is not of the same version that is referenced by this validator, or is not a schema.");
        return 0;
    }

    //Change current directory into the schema directory.
    char* original_cwd = malloc(PATH_MAX);
    if (getcwd(original_cwd, PATH_MAX) == NULL)
    {
        sprintf(error_message, "Failed fetching the current directory.");
        return 0;
    }
    if (chdir(schema_directory))
    {
        sprintf(error_message, "Failed to chdir into schema directory.");
        return 0;
    }

    //Parse the top level structure appropriately.
    int result = validate_field("parent", schema, object, error_message);

    //Change back to original CWD.
    chdir(original_cwd);
    free(original_cwd);

    return result;
}

//Validates a single JSON field given a schema/object.
int validate_field(const char* field_name, json_object* schema, json_object* object, char* error_message)
{
    //If there is a "$ref" field, attempt to load the referenced schema.
    json_object* ref_schema = json_object_object_get(schema, "$ref");
    if (ref_schema != NULL && json_object_get_type(ref_schema) == json_type_string)
    {
        //Attempt to load. If loading fails, report error.
        const char* ref_path = json_object_get_string(ref_schema);
        schema = json_object_from_file(ref_path);
        if (schema == NULL)
        {
            sprintf(error_message, "Failed to open referenced schema file '%s'.", ref_path);
            return 0;
        }
    }

    //Get the schema field type.
    json_object* desired_field_type = json_object_object_get(schema, "type");
    if (desired_field_type == NULL || !json_object_is_type(desired_field_type, json_type_string))
    {
        sprintf(error_message, "Desired field type not provided within schema/is not a string for field '%s' (schema violation).", field_name);
        return 0;
    }

    //Check the field types are actually equal.
    const char* desired_field_type_str = json_object_get_string(desired_field_type);
    if (!(
        (!strcmp(desired_field_type_str, "object") && json_object_is_type(object, json_type_object))
        || (!strcmp(desired_field_type_str, "array") && json_object_is_type(object, json_type_array))
        || (!strcmp(desired_field_type_str, "integer") && json_object_is_type(object, json_type_int))
        || (!strcmp(desired_field_type_str, "string") && json_object_is_type(object, json_type_string))
        || (!strcmp(desired_field_type_str, "boolean") && json_object_is_type(object, json_type_boolean))
        || (!strcmp(desired_field_type_str, "double") && json_object_is_type(object, json_type_double))
    ))
    {
        sprintf(error_message, "Field type match failed for field '%s'.", field_name);
        return 0;
    }

    //Switch and validate each type in turn.
    switch (json_object_get_type(object))
    {
        case json_type_int:
            return validate_integer(field_name, schema, object, error_message);
        case json_type_string:
            return validate_string(field_name, schema, object, error_message);
        case json_type_object:
            return validate_object(field_name, schema, object, error_message);
        case json_type_array:
            return validate_object(field_name, schema, object, error_message);

        //We don't perform extra validation on this type.
        default:
            return 1;
    }
}

//Validates a single integer value according to the given specification.
int validate_integer(const char* field_name, json_object* schema, json_object* object, char* error_message)
{
    //Is there a minimum/maximum specified? If so, check those.
    //Validate minimum.
    json_object* min_value = json_object_object_get(schema, "minimum");
    if (min_value != NULL && json_object_is_type(min_value, json_type_int))
    {
        int min_value_int = json_object_get_int(min_value);
        if (json_object_get_uint64(object) < min_value_int)
        {
            sprintf(error_message, "Failed to validate integer field '%s'. Value was below minimum of %d.", field_name, min_value_int);
            return 0;
        }
    }

    //Validate maximum.
    json_object* max_value = json_object_object_get(schema, "maximum");
    if (max_value != NULL && json_object_is_type(max_value, json_type_int))
    {
        int max_value_int = json_object_get_int(max_value);
        if (json_object_get_uint64(object) > max_value_int)
        {
            sprintf(error_message, "Failed to validate integer field '%s'. Value was above maximum of %d.", field_name, max_value_int);
            return 0;
        }
    }

    return 1;
}

//Validates a single string value according to the given specification.
int validate_string(const char* field_name, json_object* schema, json_object* object, char* error_message)
{
    //todo: if there is a "pattern" field, verify the string with RegEx.
    return 1;
}

//Validates a single object value according to the given specification.
int validate_object(const char* field_name, json_object* schema, json_object* object, char* error_message)
{
    //Are there a set of "required" fields? If so, check they all exist.
    json_object* required_fields = json_object_object_get(schema, "required");
    if (required_fields != NULL && json_object_get_type(required_fields) == json_type_array)
    {
        int len = json_object_array_length(required_fields);
        for (int i=0; i<len; i++)
        {
            //Get the required field from schema.
            json_object* required_field = json_object_array_get_idx(required_fields, i);
            if (json_object_get_type(required_field) != json_type_string)
            {
                sprintf(error_message, "Required field for object '%s' is not a string (schema violation).", field_name);
                return 0;
            }

            //Does it exist in the object?
            const char* required_field_str = json_object_get_string(required_field);
            if (json_object_object_get(object, required_field_str) == NULL)
            {
                sprintf(error_message, "Required field '%s' was not present in object '%s'.", required_field_str, field_name);
                return 0;
            }
        }
    }

    //If the boolean field "additionalProperties" exists and is set to false, ensure there are no
    //extra properties apart from those required in the object.
    //... todo

    //Run through the "properties" object and validate each of those in turn.
    json_object* properties = json_object_object_get(schema, "properties");
    if (properties != NULL && json_object_get_type(properties) == json_type_object)
    {
        json_object_object_foreach(properties, key, value) {

            //If the given property name does not exist on the target object, ignore and continue next.
            json_object* object_prop = json_object_object_get(object, key);
            if (object_prop == NULL)
                continue;

            //Validate against the schema.
            if (!validate_field(key, value, object_prop, error_message))
                return 0;
        }
    }

    return 1;
}

//Validates a single array value according to the given specification.
int validate_array(const char* field_name, json_object* schema, json_object* object, char* error_message)
{
    return 1;
}