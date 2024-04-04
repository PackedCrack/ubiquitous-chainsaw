//
// Created by qwerty on 2024-04-03.
//
#include "CErrorMessage.hpp"
#include "../../client_defines.hpp"
// win32
#include <strsafe.h>


namespace sys
{
CErrorMessage::CErrorMessage(DWORD code)
    : m_pMsg{ nullptr }
    , m_Size{ 0u }
{
    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-formatmessage
    // The lpBuffer parameter is a pointer to an LPTSTR; you must cast the pointer to an LPTSTR (for example, (LPTSTR)&lpBuffer).
    m_Size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                           nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&m_pMsg, 0, nullptr);
    if(m_Size == 0)
    {
        LOG_FATAL_FMT("Failed to create buffer for win32 error message. \"{}\"", GetLastError());
    }
    // If the function succeeds, the return value is the number of TCHARs stored in the output buffer,
    // EXCLUDING the terminating null character.
    m_Size = m_Size + 1;
}
CErrorMessage::~CErrorMessage()
{
    if(m_pMsg != nullptr)
    {
        // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localfree
        WIN_CHECK(HLOCAL handle = LocalFree(m_pMsg); handle == nullptr);
    }
}
CErrorMessage::CErrorMessage(const CErrorMessage& other)
    : m_pMsg{ copy(other) }
    , m_Size{ other.m_Size }
{}
CErrorMessage::CErrorMessage(CErrorMessage&& other) noexcept
    : m_pMsg{ std::exchange(other.m_pMsg, nullptr) }
    , m_Size{ other.m_Size }
{}
CErrorMessage& CErrorMessage::operator=(const CErrorMessage& other)
{
    if(this != &other)
    {
        m_pMsg = copy(other);
        m_Size = other.m_Size;
    }
    
    return *this;
}
CErrorMessage& CErrorMessage::operator=(CErrorMessage&& other) noexcept
{
    m_pMsg = std::exchange(other.m_pMsg, nullptr);
    m_Size = other.m_Size;
    
    return *this;
}
PSTR CErrorMessage::copy(const CErrorMessage& other)
{
    // https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-localalloc
    m_pMsg = static_cast<PSTR>(LocalAlloc(LMEM_FIXED, other.m_Size));
    if(m_pMsg == nullptr)
    {
        LOG_FATAL_FMT("Failed to allocate memory when copying Win32 error message. \"{}\"", GetLastError());
    }
    
    WIN_CHECK_HRESULT(StringCchCopy(m_pMsg, other.m_Size, other.m_pMsg));
    
    return m_pMsg;
}
LPTSTR CErrorMessage::message()
{
    return m_pMsg;
}
}   // namespace sys