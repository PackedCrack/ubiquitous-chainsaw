//
// Created by qwerty on 2024-03-01.
//
#include "CService.hpp"
#include "../ble_common.hpp"


namespace ble
{
CService::awaitable_t CService::make(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service)
{
    CService sv{ service };
    std::printf("\nService UUID: %ws", to_hstring(sv.m_Service.value().Uuid()).data());
    co_await sv.query_characteristics();
    
    co_return sv;
}
CService::CService(GattDeviceService service)
    : m_Service{ std::make_optional<GattDeviceService>(std::move(service)) }
    , m_Characteristics{}
    , m_State{ State::uninitialized }
{}
//winrt::Windows::Foundation::IAsyncAction CService::init()
//{
//    std::printf("\nService UUID: %ws", to_hstring(m_Service.Uuid()).data());
//
//    co_await query_characteristics();
//}
std::expected<const CCharacteristic*, CService::Error> CService::characteristic(const UUID& uuid) const
{
    auto iter = m_Characteristics.find(uuid);
    if(iter == std::end(m_Characteristics))
        return std::unexpected{ Error::characteristicNotFound };
    
    return &(iter->second);
}
std::string CService::uuid_as_str() const
{
    return winrt::to_string(winrt::to_hstring(m_Service.value().Uuid()));
}
bool CService::ready() const
{
    return m_State == State::ready;
}
CService::State CService::state() const
{
    return m_State;
}
winrt::Windows::Foundation::IAsyncAction CService::query_characteristics()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;
    
    
    m_State = State::queryingCharacteristics;
    m_Characteristics.clear();
    
    GattCharacteristicsResult result = co_await m_Service.value().GetCharacteristicsAsync();
    if(result.Status() == GattCommunicationStatus::Success)
    {
        IVectorView<GattCharacteristic> characteristics = result.Characteristics();
        m_Characteristics.reserve(characteristics.Size());
        
        for(auto&& characteristic : characteristics)
        {
            auto[iter, emplaced] = m_Characteristics.try_emplace(
                    make_uuid(characteristic.Uuid()), CCharacteristic::make_characteristic(characteristic));
            if(!emplaced)
            {
                LOG_ERROR_FMT("Failed to emplace characteristic with UUID: \"{}\"", uuid_as_str());
            }
        }
    }
    else
    {
        LOG_ERROR_FMT("Communication error: \"{}\" when trying to query Characteristics from Service with UUID: \"{}\"",
                      gatt_communication_status_to_str(result.Status()),
                      uuid_as_str());
    }
    
    m_State = State::ready;
}
}   // namespace ble