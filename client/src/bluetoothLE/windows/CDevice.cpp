//
// Created by qwerty on 2024-02-23.
//
#include "CDevice.hpp"
#include "../common.hpp"


namespace
{
void log_service_query_err(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus status)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    UNHANDLED_CASE_PROTECTION_ON
    switch(status)
    {
        case GattCommunicationStatus::Unreachable:
            LOG_ERROR("Could not retrieve service result. Reason: Unreachable");
            break;
        case GattCommunicationStatus::ProtocolError:
            LOG_ERROR("Could not retrieve service result. Reason: Protocol error");
            break;
        case GattCommunicationStatus::AccessDenied:
            LOG_ERROR("Could not retrieve service result. Reason: Access was denied");
            break;
        case GattCommunicationStatus::Success:
            std::unreachable();
    }
    UNHANDLED_CASE_PROTECTION_OFF
}
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
namespace ble::win
{
CDevice CDevice::make_device(uint64_t address)
{
    CDevice dev{};
    dev.m_State = State::uninitialized;
    
    dev.init(address);
    
    return dev;
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
winrt::Windows::Foundation::IAsyncAction CDevice::init(uint64_t address)
{
    using namespace winrt::Windows::Devices::Bluetooth;
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;
    
    /*
     * The returned BluetoothLEDevice is set to null if
     * FromBluetoothAddressAsync can't find the device identified by bluetoothAddress.
     * */
    //BluetoothLEDevice dev = co_await BluetoothLEDevice::FromBluetoothAddressAsync(address);
    BluetoothLEDevice dev = co_await BluetoothLEDevice::FromBluetoothAddressAsync(address);
    if(dev != nullptr)
    {
        GattDeviceServicesResult result = co_await dev.GetGattServicesAsync();
        if (result.Status() == GattCommunicationStatus::Success)
        {
            IVectorView<GattDeviceService> services = result.Services();
            m_Services.reserve(services.Size());
        
            for(auto&& service : services)
            {
                auto[iter, emplaced] = m_Services.try_emplace(make_uuid(service.Uuid()), CService::make_service(service));
                if(!emplaced)
                {
                    LOG_ERROR("Failed to emplace service..");
                }
                
                
            }
        }
        else
        {
            log_service_query_err(result.Status());
        
            m_State = State::serviceQueryFailed;
        }
        
        m_Device.emplace(std::move(dev));
        m_State = State::ready;
    }
    else
    {
        LOG_ERROR_FMT("Failed to instantiate CDevice: Could not find a peripheral with address: \"{}\"", ble::hex_addr_to_str(address));
        m_State = State::invalidAddress;
    }
}
}   // namespace ble::win