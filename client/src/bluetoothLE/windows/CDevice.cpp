//
// Created by qwerty on 2024-02-23.
//
#include "CDevice.hpp"
#include "../ble_common.hpp"


namespace
{
[[nodiscard]] ble::UUID make_uuid(const winrt::guid& guid)
{
    ble::UUID uuid{};
    static_assert(std::is_trivially_copyable_v<std::remove_reference_t<decltype(guid)>>);
    static_assert(std::is_trivially_copy_constructible_v<ble::UUID>);
    static_assert(sizeof(ble::UUID) == sizeof(decltype(guid)));
    std::memcpy(&uuid, &guid, sizeof(uuid));
    
    return uuid;
}
}   // namespace
namespace ble
{
CDevice::awaitable_t CDevice::make(uint64_t address)
{
    using namespace winrt::Windows::Devices::Bluetooth;
    
    CDevice dev{};
    {
        /*
     * The returned BluetoothLEDevice is set to null if
     * FromBluetoothAddressAsync can't find the device identified by bluetoothAddress.
     * */
        //BluetoothLEDevice dev = co_await BluetoothLEDevice::FromBluetoothAddressAsync(address);
        BluetoothLEDevice device = co_await BluetoothLEDevice::FromBluetoothAddressAsync(address);
        if(device != nullptr)
        {
            dev.m_Device.emplace(std::move(device));
            co_await dev.query_services();
        }
        else
        {
            dev.m_State = State::invalidAddress;
            LOG_ERROR_FMT("Failed to instantiate CDevice: Could not find a peripheral with address: \"{}\"", ble::hex_addr_to_str(address));
        }
    }
    
    co_return dev;
}
uint64_t CDevice::address() const
{
    ASSERT(m_State == State::ready && m_Device.has_value(), "Cannot retrieve address until a connection has been established.");
    
    return m_Device.value().BluetoothAddress();
}
std::string CDevice::address_as_str() const
{
    return hex_addr_to_str(address());
}
bool CDevice::ready() const
{
    return m_Device.has_value() && m_State == State::ready;
}
CDevice::State CDevice::state() const
{
    return m_State;
}
const std::unordered_map<UUID, CService, UUID::Hasher>& CDevice::services() const
{
    return m_Services;
}
winrt::Windows::Foundation::IAsyncAction CDevice::query_services()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;
    
    
    m_State = State::queryingServices;
    m_Services.clear();
    
    GattDeviceServicesResult result = co_await m_Device.value().GetGattServicesAsync();
    if (result.Status() == GattCommunicationStatus::Success)
    {
        IVectorView<GattDeviceService> services = result.Services();
        m_Services.reserve(services.Size());
        
        for(auto&& service : services)
        {
            auto[iter, emplaced] = m_Services.try_emplace(make_uuid(service.Uuid()), co_await make_service<CService>(service));
            if(!emplaced)
            {
                LOG_ERROR_FMT("Failed to emplace service with UUID: \"{}\"", winrt::to_string(to_hstring(service.Uuid())));
            }
        }
    }
    else
    {
        LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Services from device with address: \"{}\"",
                      gatt_communication_status_to_str(result.Status()),
                      hex_addr_to_str(m_Device.value().BluetoothAddress()));
    }
    
    m_State = State::ready;
}
}   // namespace ble