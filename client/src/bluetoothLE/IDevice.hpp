//
// Created by qwerty on 2024-02-14.
//
#pragma once

template<typename derived_t>
class IBLEDevice
{
protected:
    IBLEDevice() = default;
    virtual ~IBLEDevice() = default;
public:
    IBLEDevice(const IBLEDevice& other) = delete;
    IBLEDevice(IBLEDevice&& other) = delete;
    IBLEDevice& operator=(const IBLEDevice& other) = delete;
    IBLEDevice& operator=(IBLEDevice&& other) = delete;
public:
private:
};