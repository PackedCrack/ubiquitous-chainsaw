//
// Created by qwerty on 2024-03-01.
//

#include "CSystem.hpp"
#include "../../client_defines.hpp"
#include "../../resource.hpp"
// win32
#include <windows.h>
#include <shlobj_core.h>
#include <pathcch.h>
#include <sddl.h>
#include <securitybaseapi.h>
#include <powrprof.h>
#pragma comment(lib, "pathcch.lib")
#pragma comment(lib, "PowrProf.lib")
// winrt
#include <winrt/windows.foundation.h>


namespace sys
{
CSystem::CSystem()
{
    winrt::init_apartment();
}
CSystem::~CSystem()
{
    winrt::uninit_apartment();
}
/*
 *  Free functions
 */
void cowabunga()
{
    //  https://learn.microsoft.com/en-us/windows/win32/api/powrprof/nf-powrprof-setsuspendstate
    WIN_CHECK(SetSuspendState(true, false, false) == 0);
}
void auto_wakeup_timer(std::chrono::seconds&& delay)
{
    //  https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-createwaitabletimerw
    HANDLE timer = nullptr;
    WIN_CHECK(timer = CreateWaitableTimerW(nullptr, true, nullptr); timer != nullptr);
    
    
    LARGE_INTEGER time{};
    // Negative values are used for relative time (i think)
    // Time is 100 nano second intervals
    time.QuadPart = static_cast<LONGLONG>(delay.count() * 10'000'000) * -1;
    time.QuadPart = -100'000'000LL;
    //  https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-setwaitabletimer
    WIN_CHECK(SetWaitableTimer(timer, &time, 0, nullptr, nullptr, true));
}
void restrict_file_permissions(const std::filesystem::path& file)
{
    // https://learn.microsoft.com/en-us/windows/win32/secauthz/security-descriptor-string-format
    PSECURITY_DESCRIPTOR pDescriptor = nullptr;
    // https://learn.microsoft.com/en-us/windows/win32/api/sddl/nf-sddl-convertstringsecuritydescriptortosecuritydescriptorw
    WIN_CHECK(ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:P(A;OICI;FA;;;BA)", SDDL_REVISION_1, &pDescriptor, nullptr));
    if(pDescriptor != nullptr)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-setfilesecurityw
        WIN_CHECK(SetFileSecurityW(file.wstring().c_str(), DACL_SECURITY_INFORMATION, pDescriptor));
        
        // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
        WIN_CHECK(LocalFree(pDescriptor) == nullptr);
    }
}
std::expected<std::filesystem::path, std::string> key_location()
{
    std::array<WCHAR, MAX_PATH> location{ 0 };
    // https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetfolderpathw
    WIN_CHECK_HRESULT(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, NO_FLAGS, location.data()));
    
    // https://learn.microsoft.com/en-us/windows/win32/api/pathcch/nf-pathcch-pathcchappendex
    HRESULT result = PathCchAppendEx(location.data(), location.size(), L"Ubiquitous-Chainsaw\\Keys", NO_FLAGS);
    if(result != S_OK)
    {
        if(result == E_OUTOFMEMORY)
            return std::unexpected{ "Failed to append key directory to appdata path because there was not enough free memory!" };
        if(result == E_INVALIDARG)
            return std::unexpected{ "Failed to append key directory to appdata path!" };
        // This error exists according to the documentation but it is an undefined symbol.. ?
        //if(result == PATHCCH_E_FILENAME_TOO_LONG)
        //    return std::unexpected{ Error{ .msg = "Failed to append key directory to appdata path because the final filename was too long!" } };
    }
    
    return std::expected<std::filesystem::path, std::string>{ location.data() };
}
}   // namespace sys
