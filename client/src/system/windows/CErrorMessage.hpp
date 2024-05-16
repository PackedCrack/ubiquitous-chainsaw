//
// Created by qwerty on 2024-04-03.
//

#pragma once
#include "win32.hpp"
namespace sys
{
class CErrorMessage
{
#ifdef UNICODE
    using PSTR = LPWSTR;
#else
    using PSTR = LPSTR;
#endif
public:
    explicit CErrorMessage(DWORD code);
    ~CErrorMessage();
    CErrorMessage(const CErrorMessage& other);
    CErrorMessage(CErrorMessage&& other) noexcept;
    CErrorMessage& operator=(const CErrorMessage& other);
    CErrorMessage& operator=(CErrorMessage&& other) noexcept;
private:
    [[nodiscard]] PSTR copy(const CErrorMessage& other);
public:
    [[nodiscard]] LPTSTR message();
private:
    PSTR m_pMsg;
    size_t m_Size;
};
}    // namespace sys
