#include "general.h"
#include "hash_table.h"

#include "parser.h"
#include <stdlib.h>
#include <ctype.h>

#include "interp.h"
#include "schema.h"

#include "parser_private.h"
#include "bytecode_runner.h"
#include "primitives_private.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "lerp_window.h"

void update_hook() {
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
//        if (msg.message == WM_QUIT) break;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void proc_sleep(Lerp_Interp *interp, Lerp_Call_Record *record) {
    update_hook();

    bool success = argument_count(interp, record, 1);
    if (!success) return;

    float seconds;
    success = coerce_to_float(interp, record, 0, &seconds);
    if (!success) return;

    Sleep((unsigned long)(seconds * 1000));
}

void proc_do_window_events(Lerp_Interp *interp, Lerp_Call_Record *record) {
    update_hook();

    extern Window_Lib *glib;

    // Update the mouse pointer info...
    POINT cursor_point;
    BOOL success = GetCursorPos(&cursor_point);
    Schema *ns = glib->Mouse_Pointer->schema->read();

    Database *mouse = glib->Mouse_Pointer;
    if (success && ns) {
        First_Class *x_fc = mouse->lookup_named_slot(interp, glib->x_atom);
        First_Class *y_fc = mouse->lookup_named_slot(interp, glib->y_atom);
        
        if (x_fc && y_fc && (x_fc->type == ARG_INTEGER) && (y_fc->type == ARG_INTEGER)) {
            Integer *x = (Integer *)x_fc;
            Integer *y = (Integer *)y_fc;

            // @WriteBarrier @GC?
            if (cursor_point.x != x->value) mouse->assign_named_slot(interp, glib->x_atom, ToBarrierF(interp->parser->make_integer(cursor_point.x)));
            if (cursor_point.y != y->value) mouse->assign_named_slot(interp, glib->y_atom, ToBarrierF(interp->parser->make_integer(cursor_point.y)));
        }
    }
}

void proc_mouse_pointer_update(Lerp_Interp *interp, Lerp_Call_Record *record) {
    Schema *ns = glib->Mouse_Pointer->schema->read();
    if (!ns) return;

    First_Class *x_fc = glib->Mouse_Pointer->lookup_named_slot(interp, glib->x_atom);
    First_Class *y_fc = glib->Mouse_Pointer->lookup_named_slot(interp, glib->y_atom);
    if (!x_fc) return;
    if (!y_fc) return;
    if (x_fc->type != ARG_INTEGER) return;
    if (y_fc->type != ARG_INTEGER) return;

    int x = ((Integer *)x_fc)->value;
    int y = ((Integer *)y_fc)->value;

/*
    POINT cursor_point;
    cursor_point.x = x;
    cursor_point.y = y;
*/
    SetCursorPos(x, y);


    First_Class *visible_fc = glib->Mouse_Pointer->lookup_named_slot(interp, glib->visible_atom);
    if (!visible_fc) return;
    if (visible_fc->type != ARG_INTEGER) return;

    Integer *visible = (Integer *)visible_fc;
    
    int cursor_visible = ShowCursor(FALSE);

    // Annoying win32 state-holding ShowCursor.... aaaaargh
    if (visible->value) {
        if (cursor_visible < 1) {
            ShowCursor(TRUE);
            ShowCursor(TRUE);
        }
    } else {
        if (cursor_visible < -1) ShowCursor(TRUE);
    }

    glib->update_keyboard();
}


LARGE_INTEGER global_base_time;  // static global variable...
static LARGE_INTEGER get_time_reading() {
    LARGE_INTEGER freq;
    LARGE_INTEGER time;

    BOOL ok = QueryPerformanceFrequency(&freq);
    assert(ok == TRUE);

    freq.QuadPart = freq.QuadPart / 1000;

    ok = QueryPerformanceCounter(&time);
    assert(ok == TRUE);

    time.QuadPart = time.QuadPart / freq.QuadPart;

	return time;
}

void primitives_os_init() {
    global_base_time = get_time_reading();
}


void proc_seconds_since_startup(Lerp_Interp *interp, Lerp_Call_Record *record) {
    LARGE_INTEGER time = get_time_reading();
	time.QuadPart = time.QuadPart - global_base_time.QuadPart;

	Return(interp->parser->make_float((double)time.QuadPart * 0.001));
}

double get_time_in_seconds() {
    LARGE_INTEGER time = get_time_reading();
	time.QuadPart = time.QuadPart - global_base_time.QuadPart;

	return (double)(time.QuadPart * 0.001);
}
