#pragma once

#include "CGapService.hpp"


namespace nimble
{

/* This class manages the gap and gatt services */
class CGattServer 
{
public:
    CGattServer() = default;
    CGattServer(const std::string_view deviceName, const uint8_t addrType);
    ~CGattServer() = default;
    CGattServer(const CGattServer& other) = delete; 
    CGattServer(CGattServer&& other) = default; 
    CGattServer& operator=(const CGattServer& other) = delete; 
    CGattServer& operator=(CGattServer&& other) = default;  

    [[NoDiscard]] uint8_t gap_param_is_alive();
private:
//static int gap_event_handler(struct ble_gap_event *event, void *arg); 
//static void gatt_svc_register_handle(struct ble_gatt_register_ctxt *ctxt, void *arg);

private:
CGapService m_gap;
//CGattService gatt;

};

}
