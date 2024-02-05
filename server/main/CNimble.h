#pragma once
/* STD */
#include <cstdio>
#include <type_traits>

/* Project */
#include "defines.hpp"


/* BLE */
#include "nimble/nimble_port.h"
#include "host/ble_hs.h"
#include "esp_netif_ip_addr.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"

/* ESP */

#include "esp_log.h"
//#include "nimble/nimble_port_freertos.h"
//#include "console/console.h"
//#include "services/gap/ble_svc_gap.h"



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
        static void nimble_on_reset_handle(int reason);
        static void nimble_on_sync_handle(void);
        //static void gap_advertise();


        // Question: Do yo usually declare helper function in header?
        void print_adv_field_flags(const ble_hs_adv_fields& field);
        void print_adv_field_signal_power(const ble_hs_adv_fields& field);
        static void server_on_reset_handle(int reason);
        static void server_on_sync_handle(void);
        static int server_gap_on_connection_handler(struct ble_gap_event *event, void *arg);
        static void gap_advertise(void);

    } // namespace

    class CNimble 
    {

    public:
        CNimble() = delete;
        CNimble(const char* deviceName);
        ~CNimble();
        CNimble(const CNimble& other) = default;
        CNimble(CNimble&& other) = default;
        CNimble& operator=(const CNimble& other) = default;
        CNimble& operator=(CNimble&& other) = default;



    public:
    
    };

} // namespace nimble

