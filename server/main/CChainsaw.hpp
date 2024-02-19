#pragma once

/* STD */
#include <future>

/* Project */
#include "Nimble.hpp"
#include "CGap.hpp"
#include "CGatt.hpp"

namespace application
{

class CChainsaw 
{
public:
    CChainsaw();
    ~CChainsaw() = default;
    CChainsaw(const CChainsaw& other) = delete; // Copy constructor:
    CChainsaw(CChainsaw&& other) = delete; // Move constructor:
    CChainsaw& operator=(const CChainsaw& other) = delete; // copy assign
    CChainsaw& operator=(CChainsaw&& other) = delete; // move assign
public:
    void rssi();
private:
    CNimble m_nimbleHost;
    CGap m_gap;
    CGatt m_gatt;
};

}