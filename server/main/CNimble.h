#pragma once
/* STD */
#include <cstdio>

/* Project */
#include "defines.hpp"

/* BLE */
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"

//#include "nimble/nimble_port_freertos.h"
//#include "host/ble_hs.h"
//#include "host/util/util.h"
//#include "console/console.h"
//#include "services/gap/ble_svc_gap.h"



namespace nimble 
{

    namespace 
    {
        static void nimble_on_reset_cb(int reason);
        static void nimble_on_sync_cb(void);

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

