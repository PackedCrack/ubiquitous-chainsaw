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
// clang-format off


// clang-format on
namespace gui
{
CDeviceList::CDeviceList(ble::CScanner& scanner, CAuthenticator& authenticator)
    : m_pScanner{ &scanner }
    , m_pAuthenticator{ &authenticator }
    , m_Devices{}
    , m_pMutex{ std::make_unique<std::mutex>() }
    , m_ScanTimer{}
{
    spawn_time_limited_scan();
}
CDeviceList::~CDeviceList()
{
    if (m_pScanner->scanning())
    {
        {
            std::lock_guard lock{ *m_pMutex };
            if (m_ScanTimer.active())
            {
                m_ScanTimer.stop();
            }
        }

        // spin until the thread has stopped
        while (m_pScanner->scanning())
        {};
    }
}
CDeviceList::CDeviceList(const CDeviceList& other)
    : m_pScanner{ nullptr }
    , m_pAuthenticator{ nullptr }
    , m_Devices{}
    , m_pMutex{ nullptr }
    , m_ScanTimer{}
{
    copy(other);
}
CDeviceList::CDeviceList(CDeviceList&& other) noexcept
    : m_pScanner{ nullptr }
    , m_pAuthenticator{ nullptr }
    , m_Devices{}
    , m_pMutex{ nullptr }
    , m_ScanTimer{}
{
    other.m_pMutex->lock();

    m_pScanner = std::exchange(other.m_pScanner, nullptr);
    m_pAuthenticator = std::exchange(other.m_pAuthenticator, nullptr);
    m_Devices = std::move(other.m_Devices);
    m_pMutex = std::exchange(other.m_pMutex, nullptr);
    static_assert(std::is_trivially_copyable_v<decltype(other.m_ScanTimer)>);
    m_ScanTimer = other.m_ScanTimer;

    m_pMutex->unlock();
}
CDeviceList& CDeviceList::operator=(const CDeviceList& other)
{
    if (this != &other)
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
    static_assert(std::is_trivially_copyable_v<decltype(other.m_ScanTimer)>);
    m_ScanTimer = other.m_ScanTimer;

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
    m_ScanTimer = other.m_ScanTimer;
}
void CDeviceList::push()
{
    static constexpr ImGuiWindowFlags WINDOW_FLAGS = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;

    if (ImGui::Begin("DeviceList"), nullptr, WINDOW_FLAGS)
    {
        device_list();
        authentication_status();
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
        {
            std::lock_guard<mutex_t> lock{ *m_pMutex };
            m_ScanTimer = common::CStopWatch<std::chrono::seconds>{};
        }
        m_pScanner->begin_scan();
        while (m_ScanTimer.active())
        {
            if (m_ScanTimer.lap<float>() > static_cast<float>(seconds.count()) || m_pAuthenticator->server_identified())
            {
                m_ScanTimer.stop();
                m_pScanner->end_scan();
                break;
            }

            // Intentionally throttle so this thread doesnt go bananas
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            size_t foundDevices = m_pScanner->num_devices().load();
            if (prevFound < foundDevices)
            {
                std::lock_guard<mutex_t> lock{ *m_pMutex };

                std::vector<ble::DeviceInfo> infos =
                    m_pScanner->retrieve_n_devices(static_cast<int64_t>(prevFound), static_cast<int64_t>(foundDevices - prevFound));

                m_pAuthenticator->enqueue_devices(infos);
                for (auto&& info : infos)
                {
                    m_Devices.push_back(info);
                }

                prevFound = m_Devices.size();
            }
        }
    };
}
void CDeviceList::recreate_list()
{
    ASSERT(!m_pScanner->scanning(), "Already scanning!");

    std::lock_guard<mutex_t> lock{ *m_pMutex };
    if (m_ScanTimer.active())
    {
        return;
    }

    m_Devices.clear();
    spawn_time_limited_scan();
}
void CDeviceList::spawn_time_limited_scan()
{
    tf::Executor& executor = sys::executor();
    executor.silent_async(time_limited_scan(SCAN_TIME));
}
void CDeviceList::authentication_status()
{
    if (m_pScanner->scanning())
    {
        float progress = m_ScanTimer.lap<float>() / static_cast<float>(SCAN_TIME.count());
        ImGui::ProgressBar(progress, ImVec2{ -FLT_MIN, 0.0f });
    }
    else
    {
        if (!m_pAuthenticator->server_identified())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.15f, 0.15f, 1.0f), "No server authenticated");
        }
    }
}
void CDeviceList::device_list()
{
    if (m_Devices.empty())
    {
        return;
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Devices"))
    {
        std::lock_guard<mutex_t> lock{ *m_pMutex };
        int32_t deviceNum{};
        for (auto&& deviceInfo : m_Devices)
        {
            if (deviceNum == 0)
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            }

            std::string address = ble::DeviceInfo::address_as_str(deviceInfo.address.value());
            if (ImGui::TreeNode(static_cast<void*>(&deviceNum), "%s", address.c_str()))
            {
                if (m_pAuthenticator->server_identified())
                {
                    if (m_pAuthenticator->server_address() == deviceInfo.address.value())
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.36f, 0.72f, 0.0f, 1.0f), "Authenticated");
                    }
                }

                // Use SetNextItemOpen() so set the default state of a node to be open. We could
                // also use TreeNodeEx() with the ImGuiTreeNodeFlags_DefaultOpen flag to achieve the same thing!
                if (deviceNum == 0)
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                }

                ImGui::Text("%s address", ble::address_type_to_str(deviceInfo.addressType).data());

                ImGui::TreePop();
            }
            ++deviceNum;
        }
        ImGui::TreePop();
    }
}
}    // namespace gui
