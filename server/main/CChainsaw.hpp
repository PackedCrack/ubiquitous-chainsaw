#pragma once
/* STD */

#include <array>


#include "CGapService.hpp"

#include "nimble/nimble_port.h"
//#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
//#include "services/gatt/ble_svc_gatt.h"
//#include "services/ans/ble_svc_ans.h"


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
uint8_t m_bleAddressType;
CGapService m_gapService;
};

}