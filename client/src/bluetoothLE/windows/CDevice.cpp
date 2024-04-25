//
// Created by qwerty on 2024-02-23.
//
#include "CDevice.hpp"
#include "win_ble_common.hpp"
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
}    // namespace
namespace ble
{
CDevice::awaitable_t CDevice::make(uint64_t address)
{
    using namespace winrt::Windows::Devices::Bluetooth;

    std::expected<CDevice, CDevice::Error> expected{};
    expected->m_pDevice = std::make_shared<BluetoothLEDevice>(co_await BluetoothLEDevice::FromBluetoothAddressAsync(address));
    /*
    * The returned BluetoothLEDevice is set to null if
    * FromBluetoothAddressAsync can't find the device identified by bluetoothAddress.
    * */
    if (*(expected->m_pDevice))
    {
        co_await expected->query_services();
    }
    else
    {
        LOG_WARN_FMT("Failed to instantiate CDevice: Could not find a peripheral with address: \"{}\"", ble::hex_addr_to_str(address));
        co_return std::unexpected(Error::invalidAddress);
    }

    co_return expected;
}
CDevice::CDevice(const CDevice& other)
    : m_pDevice{ nullptr }
    , m_Services{}
    , m_ConnectionChanged{}
{
    copy(other);
}
CDevice::CDevice(CDevice&& other) noexcept
    : m_pDevice{ nullptr }
    , m_Services{}
    , m_ConnectionChanged{}
{
    move(other);
}
CDevice& CDevice::operator=(const CDevice& other)
{
    if (this != &other)
    {
        copy(other);
    }

    return *this;
}
CDevice& CDevice::operator=(CDevice&& other) noexcept
{
    move(other);

    return *this;
}
void CDevice::copy(const CDevice& other)
{
    m_pDevice = other.m_pDevice;
    m_Services = other.m_Services;
    if (other.m_ConnectionChanged)
    {
        this->set_connection_changed_cb(other.m_ConnectionChanged);
    }
}
void CDevice::move(CDevice& other)
{
    m_pDevice = std::move(other.m_pDevice);
    m_Services = std::move(other.m_Services);
    if (other.m_ConnectionChanged)
    {
        this->set_connection_changed_cb(std::move(other.m_ConnectionChanged));
    }
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
std::optional<const CService*> CDevice::service(const UUID& uuid) const
{
    auto iter = m_Services.find(uuid);
    if (iter == std::end(m_Services))
    {
        return std::nullopt;
    }

    return std::make_optional<const CService*>(&(iter->second));
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
            auto [iter, emplaced] = m_Services.try_emplace(make_uuid(svc.Uuid()), co_await make_service<CService>(svc));
            if (!emplaced)
            {
                LOG_ERROR_FMT("Failed to emplace service with UUID: \"{}\"", winrt::to_string(to_hstring(svc.Uuid())));
            }
        }
    }
    else
    {
        LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Services from device with address: \"{}\"",
                      gatt_communication_status_to_str(winrt_status_to_communication_status(result.Status())),
                      hex_addr_to_str(m_pDevice->BluetoothAddress()));
    }
}
}    // namespace ble
