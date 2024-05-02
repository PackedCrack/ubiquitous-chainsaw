//
// Created by qwerty on 2024-03-01.
//
#include "CService.hpp"
#include "win_ble_common.hpp"
namespace ble
{
CService::awaitable_make_t CService::make(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service)
{
    // Work around because make_shared requires a public constructor
    // But construction of CService should go through this factory function
    std::shared_ptr<CService> pService{ new CService{ service } };

#ifndef NDEBUG
    LOG_INFO_FMT("Service UUID: \"{}\"", winrt::to_string(to_hstring(pService->m_Service.Uuid())).c_str());
#endif
    co_await pService->query_characteristics();

    co_return pService;
}
CService::~CService()
{
    //if (m_Service)
    //{
    //    if (m_pService.use_count() == 1)
    //    {
    //        m_pService->Close();
    //    }
    //}
    // Unless we manually close the service to the BLE server we will not be able to access it again..
    // It will fail with 'Access Denied'.
    m_Service.Close();
}
CService::CService(GattDeviceService service)
    : m_Service{ std::move(service) }
    , m_Characteristics{}
{}
std::optional<std::weak_ptr<CCharacteristic>> CService::characteristic(const UUID& uuid) const
{
    auto iter = m_Characteristics.find(uuid);
    if (iter == std::end(m_Characteristics))
    {
        return std::nullopt;
    }

    return std::make_optional<std::weak_ptr<CCharacteristic>>(iter->second->weak_from_this());
}
std::string CService::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_Service.Uuid()));
}
//sys::fire_and_forget_t CService::unsubscribe_from_characteristic(const UUID& characteristic)
//{
//    auto iter = m_Characteristics.find(characteristic);
//    ASSERT(iter != std::end(m_Characteristics), "Tried to unsubscribe from a non existing characteristic..");
//    CommunicationStatus status = co_await iter->second->unsubscribe();
//    ASSERT_FMT(status == CommunicationStatus::success,
//               "Expected success when unsubscribing.. Error: \"{}\"",
//               communication_status_to_str(status));
//}
winrt::Windows::Foundation::IAsyncAction CService::query_characteristics()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;


    m_Characteristics.clear();

    try
    {
        GattCharacteristicsResult result = co_await m_Service.GetCharacteristicsAsync();
        if (result.Status() == GattCommunicationStatus::Success)
        {
            IVectorView<GattCharacteristic> characteristics = result.Characteristics();
            m_Characteristics.reserve(characteristics.Size());

            for (auto&& chr : characteristics)
            {
                auto [iter, emplaced] =
                    m_Characteristics.try_emplace(make_uuid(chr.Uuid()), co_await make_characteristic<CCharacteristic>(chr));
                if (!emplaced)
                {
                    LOG_ERROR_FMT("Failed to emplace characteristic with UUID: \"{}\"", uuid_as_str());
                }
            }
        }
        else
        {
            LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Characteristics from Service with UUID: \"{}\"",
                          communication_status_to_str(communication_status_from_winrt(result.Status())),
                          uuid_as_str());
        }
    }
    catch (const winrt::hresult_error& err)
    {
        LOG_WARN_FMT("Exception: \"{:X}\" - \"{}\", thrown by WinRT when trying to query Characteristics from Service: \"{}\".",
                     err.code().value,
                     winrt::to_string(winrt::to_hstring(err.message())).c_str(),
                     winrt::to_string(winrt::to_hstring(m_Service.Uuid())).c_str());
    }
    catch (...)
    {
        LOG_ERROR_FMT("Unknown Exception thrown by WinRT when trying to query Characteristics from Service: \"{}\"",
                      winrt::to_string(winrt::to_hstring(m_Service.Uuid())).c_str());
    }
}
}    // namespace ble
