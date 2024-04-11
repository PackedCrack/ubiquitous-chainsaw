//
// Created by qwerty on 2024-04-03.
//
#include "CDeviceList.hpp"
#include "../system/System.hpp"
#include "../common/CStopWatch.hpp"
// third-party
#include "imgui/imgui.h"


namespace
{
}   // namespace
namespace gui
{
CDeviceList::CDeviceList(ble::CScanner& scanner)
    : m_pScanner{ &scanner }
    , m_Devices{}
    , m_pMutex{ std::make_unique<std::mutex>() }
    , m_Timer{}
{}
CDeviceList::CDeviceList(const CDeviceList& other)
    : m_pScanner{}
    , m_Devices{}
    , m_pMutex{ nullptr }
    , m_Timer{}
{
    copy(other);
}
CDeviceList& CDeviceList::operator=(const CDeviceList& other)
{
    if(this != &other)
    {
        copy(other);
    }
    
    return *this;
}
void CDeviceList::copy(const CDeviceList& other)
{
    std::lock_guard<mutex_t> lock{ *other.m_pMutex };
    m_pScanner = other.m_pScanner;
    m_Devices = other.m_Devices;
    m_pMutex = std::make_unique<std::remove_cvref_t<decltype(*m_pMutex)>>();
    m_Timer = other.m_Timer;
}
void CDeviceList::push()
{
    static constexpr ImGuiWindowFlags WINDOW_FLAGS =
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    
    if(ImGui::Begin("DeviceList"), nullptr, WINDOW_FLAGS)
    {
        new_scan();
        device_list();
    }
    
    ImGui::End();
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
                
                for(auto&& info : infos)
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
        for(auto&& device : m_Devices)
        {
            if (deviceNum == 0)
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            
            std::string address = ble::DeviceInfo::address_as_str(device.address.value());
            if (ImGui::TreeNode(static_cast<void*>(&deviceNum), "%s", address.c_str()))
            {
                // Use SetNextItemOpen() so set the default state of a node to be open. We could
                // also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (deviceNum == 0)
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                
                ImGui::Text("%s address", ble::address_type_to_str(device.addressType).data());
                
                ImGui::TreePop();
            }
            ++deviceNum;
        }
        ImGui::TreePop();
    }
}
}   // namespace gui