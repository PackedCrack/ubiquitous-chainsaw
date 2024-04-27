//
// Created by qwerty on 2024-03-01.
//
#include "CService.hpp"
#include "win_ble_common.hpp"
namespace ble
{
CService::awaitable_t CService::make(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service)
{
    CService sv{ service };
    std::printf("\nService UUID: %ws", to_hstring(sv.m_pService->Uuid()).data());
    co_await sv.query_characteristics();

    co_return sv;
}
CService::~CService()
{
    if (m_pService)
    {
        if (m_pService.use_count() == 1)
        {
            m_pService->Close();
        }
    }
}
CService::CService(GattDeviceService service)
    : m_pService{ std::make_shared<GattDeviceService>(std::move(service)) }
    , m_Characteristics{}
{}
std::optional<const CCharacteristic*> CService::characteristic(const UUID& uuid) const
{
    auto iter = m_Characteristics.find(uuid);
    if (iter == std::end(m_Characteristics))
    {
        return std::nullopt;
    }

    return std::make_optional<const CCharacteristic*>(&(iter->second));
}
std::string CService::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_pService->Uuid()));
}
winrt::Windows::Foundation::IAsyncAction CService::query_characteristics()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;


    m_Characteristics.clear();

    GattCharacteristicsResult result = co_await m_pService->GetCharacteristicsAsync();
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
                      gatt_communication_status_to_str(communication_status_from_winrt(result.Status())),
                      uuid_as_str());
    }
}
}    // namespace ble
