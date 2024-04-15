//
// Created by qwerty on 2024-02-17.
//
#pragma once
#include <expected>


namespace security
{
class CWolfCrypt
{
public:
    enum class Error
    {
        constructorFailure
    };
public:
    [[nodiscard]] static std::expected<CWolfCrypt*, Error> instance();
    ~CWolfCrypt();
    CWolfCrypt(const CWolfCrypt& other) = delete;
    CWolfCrypt(CWolfCrypt&& other) = delete;
    CWolfCrypt& operator=(const CWolfCrypt& other) = delete;
    CWolfCrypt& operator=(CWolfCrypt&& other) = delete;
private:
    CWolfCrypt();
};
}   // namespace security