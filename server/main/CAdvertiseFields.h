#pragma once
#include "host/ble_hs_adv.h"
#include "host/ble_gap.h"
#include "services/gap/ble_svc_gap.h"

#include <string>
#include "defines.hpp"
#include <cstdio>


namespace nimble
{

    namespace 
    {
         void print_adv_field_flags(const ble_hs_adv_fields& field);
         void print_adv_field_signal_power(const ble_hs_adv_fields& field);

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
    ble_hs_adv_fields& data(); // does not modify state
    bool is_flagged(const uint8_t flag) const;

};

} //namespace Nimble
