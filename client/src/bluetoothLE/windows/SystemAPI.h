//
// Created by qwerty on 2024-03-01.
//
#pragma once


namespace ble::win
{
struct SystemAPI
{
    SystemAPI();
    ~SystemAPI();
    SystemAPI(const SystemAPI& other) = default;
    SystemAPI(SystemAPI&& other) = default;
    SystemAPI& operator=(const SystemAPI& other) = default;
    SystemAPI& operator=(SystemAPI&& other) = default;
};
}   // namespace ble::win
