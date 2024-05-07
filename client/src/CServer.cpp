//
// Created by qwerty on 2024-04-25.
//
#include "CServer.hpp"
#include "client_common.hpp"
#include "common/CCoroutineManager.hpp"
// clang-format off


// clang-format on
namespace
{};    // namespace
CServer::CServer()
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_Server{}
{}
CServer::CServer(const CServer& other)
    : m_pMutex{ std::make_unique<mutex_type>() }
    , m_Server{}
{
    std::lock_guard lock{ *other.m_pMutex };
    m_Server = other.m_Server;
}
CServer& CServer::operator=(const CServer& other)
{
    if (this != &other)
    {
        std::lock_guard lock{ *other.m_pMutex };

        m_pMutex = std::make_unique<mutex_type>();
        m_Server = other.m_Server;
    }

    return *this;
}
void CServer::grant_authentication(AuthenticatedDevice&& device)
{
    std::lock_guard lock{ *m_pMutex };
    m_Server.emplace(std::move(device));
}
void CServer::revoke_authentication()
{
    std::lock_guard lock{ *m_pMutex };
    m_Server = std::nullopt;
}
void CServer::subscribe(std::function<void(std::span<uint8_t>)>&& cb)
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](std::weak_ptr<ble::CDevice> wpDevice, std::function<void(std::span<uint8_t>)> cb) -> sys::awaitable_t<void>
    {
        std::shared_ptr<ble::CDevice> pDevice = wpDevice.lock();
        if (!pDevice)
        {
            co_return;
        }

        std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
            pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_rssi_notification());
        if (!wpCharacteristic)
        {
            LOG_ERROR("Could not find Service: \"WhereAmI\" or Characteristic: \"RSSI Notification\" when trying to subscribe.");
            co_return;
        }

        std::shared_ptr<ble::CCharacteristic> pCharacteristic = wpCharacteristic->lock();
        if (pCharacteristic)
        {
            ble::CharacteristicSubscriptionState state = co_await pCharacteristic->subscribe_to_notify(std::move(cb));
            UNHANDLED_CASE_PROTECTION_ON
            switch (state)
            {
            case ble::CharacteristicSubscriptionState::notSubscribed:
            {
                LOG_ERROR("Failed to subscribe to Characteristic: \"RSSI Notification\".");
                break;
            }
            case ble::CharacteristicSubscriptionState::inFlight:
            {
#ifndef NDEUBG
                LOG_INFO("Subscribe attempt to Characteristic: \"RSSI Notification\" was skipped because an attempt is alread in flight.");
#endif
                break;
            }
            case ble::CharacteristicSubscriptionState::subscribed:
            {
#ifndef NDEUBG
                LOG_INFO("Subscribed successfully to Characteristic: \"RSSI Notification\".");
#endif
            }
            }
            UNHANDLED_CASE_PROTECTION_OFF
        }
    };

    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        coroutineManager.fire_and_forget(coroutine, m_Server->pDevice, std::move(cb));
    }
};
void CServer::unsubscribe()
{
    auto& coroutineManager = common::coroutine_manager_instance();
    auto coroutine = [](std::weak_ptr<ble::CDevice> wpDevice) -> sys::awaitable_t<void>
    {
        std::shared_ptr<ble::CDevice> pDevice = wpDevice.lock();
        if (!pDevice)
        {
            co_return;
        }

        std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
            pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_rssi_notification());
        if (!wpCharacteristic)
        {
            LOG_ERROR("Could not find Service: \"WhereAmI\" or Characteristic: \"RSSI Notification\" when trying to unsubscribe.");
            co_return;
        }

        std::shared_ptr<ble::CCharacteristic> pCharacteristic = wpCharacteristic->lock();
        if (pCharacteristic)
        {
            ble::CharacteristicSubscriptionState state = co_await pCharacteristic->unsubscribe();
            UNHANDLED_CASE_PROTECTION_ON
            switch (state)
            {
            case ble::CharacteristicSubscriptionState::subscribed:
            {
                LOG_ERROR("Failed to unsubscribe from Characteristic: \"RSSI Notification\".");
                break;
            }
            case ble::CharacteristicSubscriptionState::inFlight:
            {
#ifndef NDEUBG
                LOG_INFO(
                    "Unsubscribe attempt to Characteristic: \"RSSI Notification\" was skipped because an attempt is alread in flight.");
#endif
                break;
            }
            case ble::CharacteristicSubscriptionState::notSubscribed:
            {
#ifndef NDEUBG
                LOG_INFO("Unsubscribed successfully from Characteristic: \"RSSI Notification\".");
#endif
            }
            }
            UNHANDLED_CASE_PROTECTION_OFF
        }
    };

    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        coroutineManager.fire_and_forget(coroutine, m_Server->pDevice);
    }
}
bool CServer::connected() const
{
    std::lock_guard lock{ *m_pMutex };
    if (m_Server)
    {
        return m_Server->pDevice->connected();
    }

    return false;
}
bool CServer::is_authenticated() const
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server.has_value();
}
sys::awaitable_t<CServer::HasSubscribedResult> CServer::has_subscribed() const
{
    // This is a coroutine and it should have its own shared_ptr to CDevice
    // in case the Server gets deauthenticated from another thread
    std::shared_ptr<ble::CDevice> pDevice{};
    {
        std::lock_guard lock{ *m_pMutex };
        if (m_Server)
        {
            pDevice = m_Server->pDevice;
        }
        else
        {
            co_return HasSubscribedResult::notAuthenticated;
        }
    }

    ASSERT(pDevice, "Should always be valid if we get here");

    std::optional<std::weak_ptr<ble::CCharacteristic>> wpCharacteristic =
        pDevice->characteristic(ble::uuid_service_whereami(), ble::uuid_characteristic_whereami_rssi_notification());
    ASSERT(wpCharacteristic, "This characteristic should always exist");

    std::shared_ptr<ble::CCharacteristic> pCharacteristic = wpCharacteristic->lock();
    if (pCharacteristic)
    {
        ble::CharacteristicSubscriptionState state = co_await pCharacteristic->has_subscribed();
        // clang-format off
        UNHANDLED_CASE_PROTECTION_ON
        switch (state)
        {
            case ble::CharacteristicSubscriptionState::subscribed: co_return HasSubscribedResult::subscribed; 
            case ble::CharacteristicSubscriptionState::notSubscribed: co_return HasSubscribedResult::notSubscribed;
            case ble::CharacteristicSubscriptionState::inFlight: co_return HasSubscribedResult::inFlight;
        }
        UNHANDLED_CASE_PROTECTION_OFF
        // clang-format off
    }

    // If we fail to take ownership of the needed characteristic.. Just return notAuthenticated.
    co_return HasSubscribedResult::notAuthenticated;
}
uint64_t CServer::server_address() const
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server->info.address.value();
}
std::string CServer::server_address_as_str() const
{
    std::lock_guard lock{ *m_pMutex };
    return ble::DeviceInfo::address_as_str(m_Server->info.address.value());
}
std::optional<std::shared_ptr<ble::CDevice>> CServer::device()
{
    std::lock_guard lock{ *m_pMutex };
    return m_Server.has_value() ? std::make_optional<std::shared_ptr<ble::CDevice>>(m_Server->pDevice) : std::nullopt;
}
