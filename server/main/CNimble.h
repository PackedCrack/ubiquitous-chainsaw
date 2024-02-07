#pragma once
/* STD */
#include <cstdio>
#include <array>

/* Project */
#include "defines.hpp"
#include "CGapService.h"
#include "CGattServer.h"

/* BLE */
#include "nimble/nimble_port.h"
#include "host/util/util.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

/* ESP */
#include "esp_log.h"

namespace nimble 
{

    // is it possible to define consts?
    // should we even have defines?
    // should these be in header or cpp?
    // Server address defines
    //#define RND_ADDR 1
    //#define PUB_ADDR 0



    namespace 
    {
        
        void server_on_reset_handle(int reason);
        void server_on_sync_handler(void);
        void server_gatt_svc_register_handle(struct ble_gatt_register_ctxt *ctxt, void *arg); // TODO: move to GATT SERVER
        //void server_host_task(void* param);
        

    } // namespace

    class CNimble 
    {

    public:
        CNimble();
        ~CNimble();
        CNimble(const CNimble& other) = default;
        CNimble(CNimble&& other) = default;
        CNimble& operator=(const CNimble& other) = default;
        CNimble& operator=(CNimble&& other) = default;

    public:
    int m_test = 1;
    private:
        void configure_nimble_host();
    };


} // namespace nimble

