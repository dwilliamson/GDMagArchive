#include "general.h"
#include "disassembler.h"
#include "printer.h"
#include "concatenator.h"
#include "bytecode_runner.h"  // @Refactor: for Lerp_Call_Record
#include "interp.h"


Lerp_Disassembler::Lerp_Disassembler(Lerp_Interp *_interp) {
    interp = _interp;
    printer = new Printer(interp);
}

Lerp_Disassembler::~Lerp_Disassembler() {
    delete printer;
}

void Lerp_Disassembler::disassemble(Lerp_Call_Record *context) {
    printer->debugger_mode = true;
    printer->indent(6);
    printer->concatenator->reset();
    int i;
    for (i = 0; i < context->num_registers; i++) {
        First_Class *value = context->registers[i]->read();
        if (value) {
            char *type_name;
            if (value->type < ARG_NUM_TYPES) {
                type_name = interp->type_atoms[value->type]->name;
            } else {
                type_name = "...custom...";
            }
            printer->concatenator->printf("(%3d): {%s}", i, type_name);
            printer->print_value(value);
            printer->concatenator->add("\n");
        } else {
//            printer->concatenator->add("<<<null>>>\n");
        }
    }

    printer->output_buffer();
    printer->debugger_mode = false;
}
