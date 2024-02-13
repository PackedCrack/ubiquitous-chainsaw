//
// Created by qwerty on 2024-02-13.
//
#include "defense_mechanism.hpp"
#include "defines.hpp"
// win32
#include <Windows.h>
#include <powrprof.h>
#pragma comment(lib, "PowrProf.lib")


namespace defense
{
void auto_wakeup_timer(std::chrono::seconds&& delay)
{
    //  https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
    HANDLE timer = CreateWaitableTimerW(nullptr, true, nullptr);
    WIN_ASSERT(timer != nullptr);
    
    
    LARGE_INTEGER time{};
    // Negative values are used for relative time (i think)
    // Time is 100 nano second intervals
    time.QuadPart = static_cast<LONGLONG>(delay.count() * 10'000'000) * -1;
    time.QuadPart = -100'000'000LL;
    //  https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setwaitabletimer
    WIN_CHECK(SetWaitableTimer(timer, &time, 0, nullptr, nullptr, true));
}

void cowabunga()
{
    //  https://learn.microsoft.com/en-us/windows/win32/api/powrprof/nf-powrprof-setsuspendstate
    WIN_CHECK(SetSuspendState(true, false, false) == 0);
}
}   // defense