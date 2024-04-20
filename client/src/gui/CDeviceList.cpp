//
// Created by qwerty on 2024-04-03.
//
#include "CDeviceList.hpp"
#include "security/CHash.hpp"
#include "security/sha.hpp"
#include "../system/System.hpp"
#include "../common/CStopWatch.hpp"
#include "../bluetoothLE/Device.hpp"
// third-party
#include "imgui/imgui.h"


namespace
{

//////////////////////////
winrt::fire_and_forget characteristic_action(const ble::CCharacteristic* pCharacteristic,
                                                 const std::vector<uint8_t>* writeData = nullptr)
    {
    if(writeData == nullptr)
    {
        ble::CCharacteristic::read_t result = co_await pCharacteristic->read_value();
        if(result)
        {
            std::vector<uint8_t> readData = result.value();
            
            std::string str{};
            str.resize(readData.size() + 1);
            std::memcpy(str.data(), readData.data(), readData.size());
            LOG_INFO_FMT("VALUE: {}", str);
        }
        else
        {
            LOG_ERROR("failed to read data");
        }
    }
    else
    {
        auto status = co_await pCharacteristic->write_data(*writeData);
        LOG_INFO_FMT("Status after write: {}", static_cast<int32_t>(status));
    }
}
winrt::fire_and_forget do_connection_test(uint64_t address)
{
    std::expected<ble::CDevice, ble::CDevice::Error> expected = co_await ble::make_device<ble::CDevice>(address);
    if(!expected)
        co_return;
    
    ble::CDevice& device = *expected;
    std::optional<const ble::CService*> service = device.service(ble::uuid_service_whoami());
    if(service)
    {
        const ble::CService* pService = service.value();
        {
            std::optional<const ble::CCharacteristic*> serverAuth =
                    pService->characteristic(ble::uuid_characteristic_server_auth());
            
            if (serverAuth)
            {
                characteristic_action(*serverAuth);
            }
            
            std::optional<const ble::CCharacteristic*> clientAuth =
                    pService->characteristic(ble::uuid_characteristic_client_auth());
            
            if (clientAuth)
            {
                std::vector<uint8_t> data{
                    'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D'
                };
                characteristic_action(*clientAuth, &data);
            }
        }
    }
}
/////////////////////////////////
}   // namespace
namespace gui
{
CDeviceList::CDeviceList(ble::CScanner& scanner, CAuthenticator& authenticator)
    : m_pScanner{ &scanner }
    , m_pAuthenticator{ &authenticator }
    , m_Devices{}
    , m_pMutex{ std::make_unique<std::mutex>() }
    , m_Timer{}
{}
CDeviceList::CDeviceList(const CDeviceList& other)
    : m_pScanner{ nullptr }
    , m_pAuthenticator{ nullptr }
    , m_Devices{}
    , m_pMutex{ nullptr }
    , m_Timer{}
{
    copy(other);
}
CDeviceList::CDeviceList(CDeviceList&& other) noexcept
    : m_pScanner{ nullptr }
    , m_pAuthenticator{ nullptr }
    , m_Devices{}
    , m_pMutex{ nullptr }
{
    other.m_pMutex->lock();
    
    m_pScanner = std::exchange(other.m_pScanner, nullptr);
    m_pAuthenticator = std::exchange(other.m_pAuthenticator, nullptr);
    m_Devices = std::move(other.m_Devices);
    m_pMutex = std::exchange(other.m_pMutex, nullptr);
    
    m_pMutex->unlock();
}
CDeviceList& CDeviceList::operator=(const CDeviceList& other)
{
    if(this != &other)
    {
        copy(other);
    }
    
    return *this;
}
CDeviceList& CDeviceList::operator=(CDeviceList&& other) noexcept
{
    other.m_pMutex->lock();
    
    m_pScanner = std::exchange(other.m_pScanner, nullptr);
    m_pAuthenticator = std::exchange(other.m_pAuthenticator, nullptr);
    m_Devices = std::move(other.m_Devices);
    m_pMutex = std::exchange(other.m_pMutex, nullptr);
    
    m_pMutex->unlock();
    return *this;
}
void CDeviceList::copy(const CDeviceList& other)
{
    std::lock_guard<mutex_t> lock{ *other.m_pMutex };
    m_pScanner = other.m_pScanner;
    m_pAuthenticator = other.m_pAuthenticator;
    m_Devices = other.m_Devices;
    m_pMutex = std::make_unique<std::remove_cvref_t<decltype(*m_pMutex)>>();
    m_Timer = other.m_Timer;
}
void CDeviceList::push()
{
    static constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    
    if(ImGui::Begin("DeviceList"), nullptr, WINDOW_FLAGS)
    {
        device_list();
        new_scan();
    }
    
    ImGui::End();
}
std::vector<ble::DeviceInfo> CDeviceList::device_infos() const
{
    std::lock_guard lock{ *m_pMutex };
    return std::vector<ble::DeviceInfo>{ std::begin(m_Devices), std::end(m_Devices) };
}
auto CDeviceList::time_limited_scan(std::chrono::seconds seconds)
{
    return [this, seconds]()
    {
        size_t prevFound = 0;
        m_Timer.reset();
        m_pScanner->begin_scan();
        while(m_Timer.lap<float>() <= static_cast<float>(seconds.count()))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            
            size_t foundDevices = m_pScanner->num_devices().load();
            if(prevFound < foundDevices)
            {
                std::lock_guard<mutex_t> lock{ *m_pMutex };
                
                std::vector<ble::DeviceInfo> infos = m_pScanner->retrieve_n_devices(
                        static_cast<int64_t>(prevFound),
                        static_cast<int64_t>(foundDevices - prevFound));
                
                m_pAuthenticator->enqueue_devices(infos);
                for (auto&& info : infos)
                    m_Devices.push_back(info);
                
                prevFound = m_Devices.size();
            }
        }
        m_pScanner->end_scan();
    };
}
void CDeviceList::new_scan()
{
    std::chrono::seconds scanTime{ 10 };
    
    if(m_pScanner->scanning())
    {
        float progress = m_Timer.lap<float>() / static_cast<float>(scanTime.count());
        ImGui::ProgressBar(progress, ImVec2{ -FLT_MIN, 0.0f });
    }
    else
    {
        std::lock_guard<mutex_t> lock{ *m_pMutex };
        if(ImGui::Button("Start scan", ImVec2{ -FLT_MIN, 0.0f }))
        {
            m_Devices.clear();
            
            tf::Executor& executor = sys::executor();
            executor.silent_async(time_limited_scan(scanTime));
        }
    }
}
void CDeviceList::device_list()
{
    if(m_Devices.empty())
        return;
    
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Devices"))
    {
        std::lock_guard<mutex_t> lock{ *m_pMutex };
        int32_t deviceNum{};
        for(auto&& deviceInfo : m_Devices)
        {
            if (deviceNum == 0)
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            
            std::string address = ble::DeviceInfo::address_as_str(deviceInfo.address.value());
            if (ImGui::TreeNode(static_cast<void*>(&deviceNum), "%s", address.c_str()))
            {
                if (m_pAuthenticator->server_identified())
                {
                    if (m_pAuthenticator->server_address() == address)
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.36f, 0.72f, 0.0f, 0.55f), "Authenticated");
                    }
                }

                ImGui::SameLine();
                if(ImGui::Button("Connect"))
                    do_connection_test(deviceInfo.address.value());
                
                
                // Use SetNextItemOpen() so set the default state of a node to be open. We could
                // also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (deviceNum == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                
                ImGui::Text("%s address", ble::address_type_to_str(deviceInfo.addressType).data());
                
                ImGui::TreePop();
            }
            ++deviceNum;
        }
        ImGui::TreePop();
    }
}
}   // namespace gui