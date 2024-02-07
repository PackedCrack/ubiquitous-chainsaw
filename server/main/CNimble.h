#pragma once
/* STD */
#include <cstdio>

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
        void server_gatt_svc_register_handle(struct ble_gatt_register_ctxt *ctxt, void *arg);
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

    private:
        void configure_nimble_stack();
    };


} // namespace nimble

