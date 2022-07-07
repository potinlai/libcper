/**
 * Describes functions for parsing JSON IR CPER data into binary CPER format.
 * 
 * Author: Lawrence.Tang@arm.com
 **/

#include <stdio.h>
#include "json.h"
#include "cper-parse.h"

//Converts the given JSON IR CPER representation into CPER binary format, piped to the provided file stream.
void ir_to_cper(json_object* ir, FILE* out)
{
    //...
}