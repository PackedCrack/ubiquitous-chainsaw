//
// Created by qwerty on 2024-03-01.
//
#pragma once


namespace sys
{
class CSystem
{
public:
    CSystem();
    ~CSystem();
    CSystem(const CSystem& other) = delete;
    CSystem(CSystem&& other) = delete;
    CSystem& operator=(const CSystem& other) = delete;
    CSystem& operator=(CSystem&& other) = delete;
};
}   // namespace sys
