#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"
#include "module_helper.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void proc_sleep(Lerp_Interp *interp, Lerp_Call_Record *record) {
    interp->process_os_events();

    bool success = argument_count(interp, record, 1);
    if (!success) return;

    float seconds;
    success = coerce_to_float(interp, record, 0, &seconds);
    if (!success) return;

    Sleep((unsigned long)(seconds * 1000));
}


