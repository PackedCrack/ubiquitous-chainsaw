//
// Created by qwerty on 2024-04-10.
//
#include "CRSSIPlot.hpp"
// third party
#include "imgui/imgui.h"
// clang-format off


// clang-format on
namespace gui
{
CRSSIPlot::CRSSIPlot(std::size_t size, std::shared_ptr<CRssiDemander> pDemander)
    : m_Index{ 0u }
    , m_Values{}
    , m_pDemander{ std::move(pDemander) }
{
    m_Values.resize(size);
}
void CRSSIPlot::push()
{
    static constexpr ImGuiWindowFlags WINDOW_FLAGS =
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("RSSIPlot"), nullptr, WINDOW_FLAGS)
    {
        plot();
    }

    ImGui::End();
}
float CRSSIPlot::rssi_avg() const
{
    float sum = std::accumulate(std::begin(m_Values), std::end(m_Values), 0.0f);
    return sum / static_cast<float>(m_Values.size());
}
void CRSSIPlot::plot()
{
    m_pDemander->send_demand();

    std::optional<std::vector<int8_t>> rssi = m_pDemander->rssi();
    if (rssi)
    {
        for (auto&& value : rssi.value())
        {
            add_rssi_value(value);
        }
    }

    ImGui::PlotLines("RSSI",
                     m_Values.data(),
                     std::ssize(m_Values),
                     0,
                     std::format("Average: {:.2f}", rssi_avg()).c_str(),
                     -1.0f,
                     1.0f,
                     ImVec2{ -FLT_MIN, 100.0f },
                     sizeof(typename decltype(m_Values)::value_type));
}
void CRSSIPlot::add_rssi_value(float value)
{
    size_t index = m_Index % m_Values.size();
    m_Values[index] = value;
    ++m_Index;
}
}    // namespace gui
