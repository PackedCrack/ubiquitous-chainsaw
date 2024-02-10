#pragma once
/* STD */
#include <array>

/* Project */
#include "CGapService.hpp"
#include "CGattService.hpp"

/* Nimble */
#include "nimble/nimble_port.h"
#include "host/util/util.h"


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
    void start();
private:
    CGapService m_gapService;
    CGattService m_gattService;
};

}