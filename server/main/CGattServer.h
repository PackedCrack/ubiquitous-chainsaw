#pragma once

#include "CGapService.h"


namespace nimble
{

    namespace
    {
        

    }

 class CGattServer 
    {

    public:
        CGattServer() = delete;
        CGattServer(const char* deviceName, const uint8_t addrType);
        ~CGattServer() = default;
        CGattServer(const CGattServer& other) = default;
        CGattServer(CGattServer&& other) = default;
        CGattServer& operator=(const CGattServer& other) = default;
        CGattServer& operator=(CGattServer&& other) = default;


    private:
    int gap_cb_handler(struct ble_gap_event *event, void *arg); 


    public:
    CGapService m_gap;
    //CGattService gatt;

    
    


    };

}
