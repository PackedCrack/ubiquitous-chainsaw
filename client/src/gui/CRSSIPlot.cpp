//
// Created by qwerty on 2024-04-10.
//
#include "CRSSIPlot.hpp"
// third party
#include "imgui/imgui.h"
//
//
//
//
namespace gui
{
CRSSIPlot::CRSSIPlot(std::shared_ptr<CRssiDemander> pDemander, std::size_t size)
    : m_Index{ 0u }
    , m_MaxSize{ size }
    , m_Values{ 0 }
    , m_pDemander{ std::move(pDemander) }
{}
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
int8_t CRSSIPlot::rssi_median() const
{
    std::vector<float> values = m_Values;
    std::sort(std::begin(values), std::end(values));

    auto nthElement = std::begin(values) + std::size(values) / 2;
    std::nth_element(std::begin(values), nthElement, std::end(values));

    return static_cast<int8_t>(values[std::ssize(values) / 2]);
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

    static constexpr float MIN_RANGE = -128.0f;
    static constexpr float MAX_RANGE = 0.0f;
    ImGui::PlotLines("RSSI",
                     m_Values.data(),
                     std::ssize(m_Values),
                     0,
                     std::format("Median: {}", rssi_median()).c_str(),
                     MIN_RANGE,
                     MAX_RANGE,
                     ImVec2{ -FLT_MIN, 100.0f },
                     sizeof(typename decltype(m_Values)::value_type));
}
void CRSSIPlot::add_rssi_value(int8_t value)
{
    if (m_Values.front() == 0)
    {
        m_Values[0] = static_cast<float>(value);
    }
    else if (m_Values.size() < m_MaxSize)
    {
        m_Values.push_back(static_cast<float>(value));
    }
    else
    {
        // A vector is used as a ring buffer because ImGui requires a vector for PlotLines
        // Since it's a vector we can't pop_front like if we had a list
        // If we just use the [] operator to index into the correct spot in the ring buffer using %
        // then the redraw of ImGui will be incorrect. In order to make ImGui draw the line plot
        // and have it look like its moving from right to left we need to keep changing the last value.
        // Thus we need to take all but the first values in our vector and make a new vector, and then
        // finally push back the new RSSI value.
        m_Values = std::vector<float>{ std::begin(m_Values) + 1, std::end(m_Values) };
        m_Values.push_back(value);
    }
}
}    // namespace gui
