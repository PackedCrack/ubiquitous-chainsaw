#pragma once
/* STD */
#include <cstdio>

/* Project */
#include "defines.hpp"

#include "esp_log.h"


/* BLE */
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "esp_netif_ip_addr.h"
#include "host/util/util.h"

//#include "nimble/nimble_port_freertos.h"
//#include "host/ble_hs.h"
//#include "host/util/util.h"
//#include "console/console.h"
//#include "services/gap/ble_svc_gap.h"



namespace nimble 
{

    // is it possible to define consts?
    // should we even have defines?
    // should these be in header or cpp?
    // Server address defines
    #define RND_ADDR 1
    #define PUB_ADDR 0



    namespace 
    {
        static void nimble_on_reset_handle(int reason);
        static void nimble_on_sync_handle(void);

    } // namespace

    class CNimble 
    {

    public:
        CNimble();
        ~CNimble() = default;
        CNimble(const CNimble& other) = default;
        CNimble(CNimble&& other) = default;
        CNimble& operator=(const CNimble& other) = default;
        CNimble& operator=(CNimble&& other) = default;



    private:
    
    };

} // namespace nimble

