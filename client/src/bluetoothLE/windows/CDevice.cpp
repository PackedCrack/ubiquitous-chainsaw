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
CDevice::CDevice(const CDevice& other)
    : m_pDevice{ other.m_pDevice }
    , m_Services{ other.m_Services }
    , m_ConnectionChanged{ other.m_ConnectionChanged }
    , m_Revoker{}
{
    refresh_connection_changed_handler();
}
CDevice::CDevice(CDevice&& other) noexcept
    : m_pDevice{ std::move(other.m_pDevice) }
    , m_Services{ std::move(other.m_Services) }
    , m_ConnectionChanged{ std::move(other.m_ConnectionChanged) }
    , m_Revoker{ std::move(other.m_Revoker) }
{
    refresh_connection_changed_handler();
}
CDevice& CDevice::operator=(const CDevice& other)
{
    if (this != &other)
    {
        m_pDevice = other.m_pDevice;
        m_Services = other.m_Services;
        m_ConnectionChanged = other.m_ConnectionChanged;
        m_Revoker = BluetoothLEDevice::ConnectionStatusChanged_revoker{};
        refresh_connection_changed_handler();
    }

    return *this;
}
CDevice& CDevice::operator=(CDevice&& other) noexcept
{
    m_pDevice = std::move(other.m_pDevice);
    m_Services = std::move(other.m_Services);
    m_ConnectionChanged = std::move(other.m_ConnectionChanged);
    m_Revoker = std::move(other.m_Revoker);
    refresh_connection_changed_handler();

    return *this;
}
bool CDevice::connected() const
{
    return m_pDevice->ConnectionStatus() == winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Connected;
}
uint64_t CDevice::address() const
{
    ASSERT(m_pDevice != nullptr, "Cannot retrieve address until a connection has been established.");

    return m_pDevice->BluetoothAddress();
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
void CDevice::unsubscribe_from_characteristic(const ble::UUID& service, const ble::UUID& characteristic)
{
    auto iter = m_Services.find(service);
    ASSERT(iter != std::end(m_Services), "Tried to unsubscribe from a non existing service..");
    iter->second->unsubscribe_from_characteristic(characteristic);
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
    ASSERT(m_pDevice, "Should never be nullptr");
    m_Revoker = m_pDevice->ConnectionStatusChanged(winrt::auto_revoke, connection_changed_handler());
}
void CDevice::refresh_connection_changed_handler()
{
    revoke_connection_changed_handler();
    register_connection_changed_handler();
}
std::function<void(const winrt::Windows::Devices::Bluetooth::BluetoothLEDevice& device,
                   [[maybe_unused]] const winrt::Windows::Foundation::IInspectable& inspectable)>
    CDevice::connection_changed_handler()
{
    return [this](const BluetoothLEDevice& device, [[maybe_unused]] const IInspectable& inspectable)
    {
        using Status = winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus;

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

        if (m_ConnectionChanged)
        {
            this->m_ConnectionChanged(conStatus);
        }
        else
        {
            ASSERT(false, "Expected a connection changed callback");
        }
    };
}
winrt::Windows::Foundation::IAsyncAction CDevice::query_services()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;


    m_Services.clear();

    GattDeviceServicesResult result = co_await m_pDevice->GetGattServicesAsync();
    if (result.Status() == GattCommunicationStatus::Success)
    {
        IVectorView<GattDeviceService> svcs = result.Services();
        m_Services.reserve(svcs.Size());

        for (auto&& svc : svcs)
        {
            auto [iter, emplaced] =
                m_Services.try_emplace(make_uuid(svc.Uuid()), std::make_shared<CService>(co_await make_service<CService>(svc)));
            if (!emplaced)
            {
                LOG_ERROR_FMT("Failed to emplace service with UUID: \"{}\"", winrt::to_string(to_hstring(svc.Uuid())));
            }
        }
    }
    else
    {
        LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Services from device with address: \"{}\"",
                      communication_status_to_str(communication_status_from_winrt(result.Status())),
                      hex_addr_to_str(m_pDevice->BluetoothAddress()));
    }
}
}    // namespace ble
