//
// Created by qwerty on 2024-03-01.
//
#pragma once
#include "../../client_defines.hpp"
#include "../ble_common.hpp"
// winrt
#pragma warning(push)
#pragma warning(disable: 4'265)    // missing virtual destructor - wtf microsfot?
#include <winrt/Windows.Foundation.h>
#pragma warning(pop)
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <pplawait.h>
// clang-format off


// clang-format on
namespace ble
{
class CDescriptor
{
public:
    using awaitable_t = concurrency::task<CDescriptor>;
private:
    using GattDescriptor = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor;
public:
    [[nodiscard]] static awaitable_t make(const GattDescriptor& descriptor);
    CDescriptor() = default;
    ~CDescriptor() = default;
    CDescriptor(const CDescriptor& other) = default;
    CDescriptor(CDescriptor&& other) = default;
    CDescriptor& operator=(const CDescriptor& other) = default;
    CDescriptor& operator=(CDescriptor&& other) = default;
private:
    explicit CDescriptor(GattDescriptor descriptor);
public:
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] ProtectionLevel protection_level() const;
private:
    std::shared_ptr<GattDescriptor> m_pDescriptor;
};
}    // namespace ble
