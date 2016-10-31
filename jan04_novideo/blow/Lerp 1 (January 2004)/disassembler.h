struct Printer;

struct Lerp_Disassembler {
    Lerp_Disassembler(Lerp_Interp *interp);
    ~Lerp_Disassembler();

    void disassemble(Lerp_Call_Record *context);

    Lerp_Interp *interp;
    Printer *printer;
};
