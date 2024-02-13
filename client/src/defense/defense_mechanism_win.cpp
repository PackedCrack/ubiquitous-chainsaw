//
// Created by qwerty on 2024-02-13.
//
#include "defense_mechanism.hpp"
#include "defines.hpp"
// win32
#include <Windows.h>
#include <shlobj_core.h>
#include <pathcch.h>
#include <sddl.h>
#include <securitybaseapi.h>
#include <powrprof.h>
#pragma comment(lib, "pathcch.lib")
#pragma comment(lib, "PowrProf.lib")


namespace sys::defense
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
}   // sys::defense


namespace sys::files
{
void restrict_file_permissions(const std::filesystem::path& file)
{
    // https://learn.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format
    PSECURITY_DESCRIPTOR pDescriptor = nullptr;
    // https://learn.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsecuritydescriptortosecuritydescriptorw
    WIN_CHECK(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:PAI(A;;FA;;;OW)", SDDL_REVISION_1, &pDescriptor, nullptr));
    if(pDescriptor != nullptr)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-setfilesecurityw
        WIN_CHECK(SetFileSecurityW(file.wstring().c_str(), DACL_SECURITY_INFORMATION, pDescriptor));
    
        // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
        WIN_CHECK(LocalFree(pDescriptor) == nullptr);
    }
}
std::expected<std::filesystem::path, Error> key_location()
{
    std::array<WCHAR, MAX_PATH> location{ 0 };
    // https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetfolderpathw
    WIN_CHECK_HRESULT(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, NO_FLAGS, location.data()));

    // https://learn.microsoft.com/en-us/windows/win32/api/pathcch/nf-pathcch-pathcchappendex
    HRESULT result = PathCchAppendEx(location.data(), location.size(), L"Ubiquitous-Chainsaw\\Keys", NO_FLAGS);
    if(result != S_OK)
    {
        if(result == E_OUTOFMEMORY)
            return std::unexpected{ Error{ .msg = "Failed to append key directory to appdata path because there was not enough free memory!" } };
        if(result == E_INVALIDARG)
            return std::unexpected{ Error{ .msg = "Failed to append key directory to appdata path!" } };
        // This error exists according to the documentation but it is an undefined symbol.. ?
        //if(result == PATHCCH_E_FILENAME_TOO_LONG)
        //    return std::unexpected{ Error{ .msg = "Failed to append key directory to appdata path because the final filename was too long!" } };
    }
    
    return std::filesystem::path{ location.data() };
}
}   // sys::files