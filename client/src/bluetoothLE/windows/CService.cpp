//
// Created by qwerty on 2024-03-01.
//
#include "CService.hpp"
#include "../common.hpp"


namespace ble::win
{

CService CService::make_service(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService& service)
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    
    CService sv{ service };
    sv.init();
    
    return sv;
}
CService::CService(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService service)
    : m_Service{ std::move(service) }
{}
winrt::Windows::Foundation::IAsyncAction CService::init()
{
    using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
    using namespace winrt::Windows::Foundation::Collections;
    
    std::printf("\nService UUID: %ws", to_hstring(m_Service.Uuid()).data());
    
    
    GattCharacteristicsResult result = co_await m_Service.GetCharacteristicsAsync();
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
                LOG_ERROR("Failed to emplace service..");
            }
            
            
        }
    }
    else
    {
    
    }
}
}   // namespace ble::win