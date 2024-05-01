//
// Created by qwerty on 2024-02-23.
//
#include "CDevice.hpp"
#include "win_ble_common.hpp"
// clang-format off


// clang-format on
namespace ble
{
CDevice::~CDevice()
{
    revoke_connection_changed_handler();
}
CDevice::CDevice(CDevice&& other) noexcept
    : m_Device{ std::move(other.m_Device) }
    , m_Services{ std::move(other.m_Services) }
    , m_ConnectionChanged{ std::move(other.m_ConnectionChanged) }
    , m_Revoker{ std::move(other.m_Revoker) }
{}
CDevice& CDevice::operator=(CDevice&& other) noexcept
{
    m_Device = std::move(other.m_Device);
    m_Services = std::move(other.m_Services);
    m_ConnectionChanged = std::move(other.m_ConnectionChanged);
    m_Revoker = std::move(other.m_Revoker);

    return *this;
}
std::string_view CDevice::error_to_str(Error err)
{
    // clang-format off
    UNHANDLED_CASE_PROTECTION_ON
    switch (err)
    {
    case Error::invalidAddress: return "Invalid Address";
    case Error::failedToQueryServices: return "Failed To Query Services";
    case Error::constructorError: return "Constructor Error";
    }
    UNHANDLED_CASE_PROTECTION_OFF
    // clang-format on

    std::unreachable();
}
bool CDevice::connected() const
{
    ASSERT(m_Device, "Expected a valid BluetoothLEDevice!");
    return m_Device.ConnectionStatus() == winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Connected;
}
uint64_t CDevice::address() const
{
    ASSERT(m_Device, "Cannot retrieve address until a connection has been established.");
    return m_Device.BluetoothAddress();
}
std::string CDevice::address_as_str() const
{
    return hex_addr_to_str(address());
}
const CDevice::service_container_t& CDevice::services() const
{
    return m_Services;
}
std::optional<std::weak_ptr<CService>> CDevice::service(const UUID& uuid) const
{
    auto iter = m_Services.find(uuid);
    if (iter == std::end(m_Services))
    {
        return std::nullopt;
    }

    return std::make_optional<std::weak_ptr<CService>>(iter->second);
}
//void CDevice::unsubscribe_from_characteristic(const ble::UUID& service, const ble::UUID& characteristic)
//{
//    auto iter = m_Services.find(service);
//    ASSERT(iter != std::end(m_Services), "Tried to unsubscribe from a non existing service..");
//    iter->second->unsubscribe_from_characteristic(characteristic);
//}
std::function<void(const winrt::Windows::Devices::Bluetooth::BluetoothLEDevice& device,
                   const winrt::Windows::Foundation::IInspectable& inspectable)>
    CDevice::connection_changed_handler()
{
    return [wpSelf = weak_from_this(), address = address_as_str()](const BluetoothLEDevice& device,
                                                                   [[maybe_unused]] const IInspectable& inspectable)
    {
        using Status = winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus;

        try
        {
            Status status = device.ConnectionStatus();
            ConnectionStatus conStatus{};
            if (status == Status::Connected)
            {
                conStatus = ConnectionStatus::connected;
            }
            else if (status == Status::Disconnected)
            {
                conStatus = ConnectionStatus::disconnected;
            }
            else
            {
                ASSERT(false, "Unexpected value returned from ConnectionStatus");
            }

            std::shared_ptr<CDevice> pSelf = wpSelf.lock();
            if (pSelf)
            {
                ASSERT(pSelf->m_ConnectionChanged, "Expected a connection changed callback");
                pSelf->m_ConnectionChanged(conStatus);
            }
        }
        catch (const winrt::hresult_error& err)
        {
            LOG_WARN_FMT(
                "Exception: \"{:X}\" - \"{}\", thrown by WinRT during Connection Changed Callback from Device with address: \"{}\".",
                err.code().value,
                winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                address.c_str());
        }
        catch (...)
        {
            LOG_ERROR_FMT("Unknown Exception thrown during Connection Changed Callback from Device with address: \"{}\"", address.c_str());
        }
    };
}
concurrency::task<bool> CDevice::query_services(uint64_t address)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;


    m_Services.clear();

    try
    {
        GattDeviceServicesResult result = co_await m_Device.GetGattServicesAsync();
        if (result.Status() == GattCommunicationStatus::Success)
        {
            IVectorView<GattDeviceService> svcs = result.Services();
            m_Services.reserve(svcs.Size());

            for (auto&& svc : svcs)
            {
                auto [iter, emplaced] = m_Services.try_emplace(make_uuid(svc.Uuid()), co_await make_service<CService>(svc));
                ASSERT_FMT(emplaced, "Failed to emplace service with UUID: \"{}\"", winrt::to_string(to_hstring(svc.Uuid())));
            }

            co_return true;
        }
        else
        {
            LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Services from device with address: \"{}\"",
                          communication_status_to_str(communication_status_from_winrt(result.Status())),
                          hex_addr_to_str(m_Device.BluetoothAddress()));
        }
    }
    catch (const winrt::hresult_error& err)
    {
        LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to query Services from Device with address: \"{}\".",
                     err.code().value,
                     winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                     hex_addr_to_str(address).c_str());
    }
    catch (...)
    {
        LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to query Services from Device with address: \"{}\"",
                      hex_addr_to_str(address).c_str());
    }

    co_return false;
}
void CDevice::revoke_connection_changed_handler()
{
    if (m_Revoker)
    {
        m_Revoker.revoke();
    }
}
void CDevice::register_connection_changed_handler()
{
    ASSERT(m_Device, "Should never be invalid!");
    m_Revoker = m_Device.ConnectionStatusChanged(winrt::auto_revoke, connection_changed_handler());
}
}    // namespace ble
