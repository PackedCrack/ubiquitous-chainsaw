#pragma once


/* Project */
#include "defines.hpp"
//#include "CAdvertiseFields.h"
//#include "CAdvertiseParams.h"

#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"
#include "host/ble_uuid.h"

#include <string>
#include "defines.hpp"
#include <cstdio>


namespace nimble
{
    namespace
    {
        // https://www.bluetooth.com/wp-content/uploads/Files/Specification/HTML/Assigned_Numbers/out/en/Assigned_Numbers.pdf?v=1707124555335
        #define GATT_SVR_SVC_ATTRI_UUID 0x1801
        #define GATT_SVR_SVC_ALERT_UUID 0x1811

    } // namespace

    class CAdvertiseFields {

    public:
        CAdvertiseFields() = delete;
        CAdvertiseFields(const char* deviceName);
        ~CAdvertiseFields() = default;
        CAdvertiseFields(const CAdvertiseFields& other) = default;
        CAdvertiseFields(CAdvertiseFields&& other) = default;
        CAdvertiseFields& operator=(const CAdvertiseFields& other) = default;
        CAdvertiseFields& operator=(CAdvertiseFields&& other) = default;

    private:
    ble_hs_adv_fields m_fields;

    public:
    const ble_hs_adv_fields& data() const; 
    bool is_flagged(const uint8_t flag) const;
    int configure_fields() const;

    };

    class CAdvertiseParams {
    public:
        CAdvertiseParams();
        ~CAdvertiseParams() = default;
        CAdvertiseParams(const CAdvertiseParams& other) = default;
        CAdvertiseParams(CAdvertiseParams&& other) = default;
        CAdvertiseParams& operator=(const CAdvertiseParams& other) = default;
        CAdvertiseParams& operator=(CAdvertiseParams&& other) = default;

    private:
    ble_gap_adv_params m_params;

    public:
    const ble_gap_adv_params& data() const;

    };

    class CGapService {

        // function ptr for gap callback event
        typedef int (*GapEventHandler)(struct ble_gap_event*, void*arg);

        public:
            CGapService() = delete;
            CGapService(const char* deviceName, const uint8_t addrType);
            ~CGapService() = default;
            CGapService(const CGapService& other) = default;
            CGapService(CGapService&& other) = default;
            CGapService& operator=(const CGapService& other) = default;
            CGapService& operator=(CGapService&& other) = default;

        private:
        const uint8_t m_bleAddressType;
        CAdvertiseFields m_fields;
        CAdvertiseParams m_params;

        public:
            void advertise(GapEventHandler handler);
            void test(GapEventHandler handler);

        private:
            int gap_cb_handler(struct ble_gap_event *event, void *arg);
    };




} // namespace nimble
