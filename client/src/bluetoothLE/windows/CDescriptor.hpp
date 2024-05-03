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
class CDescriptor : public std::enable_shared_from_this<CDescriptor>
{
public:
    // We return shared ptrs because concurrency::task<T> requires that T has a copy constructor
    using awaitable_make_t = concurrency::task<std::shared_ptr<CDescriptor>>;
    using read_t = std::expected<std::vector<uint8_t>, CommunicationStatus>;
    using awaitable_read_t = concurrency::task<read_t>;
private:
    using GattDescriptor = winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor;
public:
    [[nodiscard]] static awaitable_make_t make(const GattDescriptor& descriptor);
    ~CDescriptor() = default;
    CDescriptor(const CDescriptor& other) = delete;
    CDescriptor(CDescriptor&& other) = default;
    CDescriptor& operator=(const CDescriptor& other) = delete;
    CDescriptor& operator=(CDescriptor&& other) = default;
private:
    explicit CDescriptor(GattDescriptor descriptor);
public:
    [[nodiscard]] awaitable_read_t read_value() const;
    [[nodiscard]] std::string uuid_as_str() const;
    [[nodiscard]] ProtectionLevel protection_level() const;
private:
    GattDescriptor m_Descriptor;
};
}    // namespace ble
