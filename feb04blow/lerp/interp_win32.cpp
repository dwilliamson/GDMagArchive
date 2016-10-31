#include "general.h"
#include "interp.h"


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct Lerp_Interp_Os_Specific {
    LARGE_INTEGER base_time;
};

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

void Lerp_Interp::init_os_specific() {
    os_specific = new Lerp_Interp_Os_Specific();  // @Leak
    os_specific->base_time = get_time_reading();    
}

double Lerp_Interp::get_time_in_seconds() {
    LARGE_INTEGER time = get_time_reading();
	time.QuadPart = time.QuadPart - os_specific->base_time.QuadPart;

	return (double)(time.QuadPart * 0.001);
}

void Lerp_Interp::process_os_events() {
    MSG msg;

    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

