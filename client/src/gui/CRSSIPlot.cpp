//
// Created by qwerty on 2024-04-10.
//
#include "CRSSIPlot.hpp"
// third party
#include "imgui/imgui.h"

namespace gui
{
CRSSIPlot::CRSSIPlot(size_t size)
    : m_Index{ 0u }
    , m_Values{ 0.0f }
{
    m_Values.resize(size);
}
void CRSSIPlot::push() const
{
    static constexpr ImGuiWindowFlags WINDOW_FLAGS =
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;
    
    if(ImGui::Begin("RSSIPlot"), nullptr, WINDOW_FLAGS)
    {
        //ImGui::PlotLines("Lines", func, NULL, display_count, 0, NULL, -1.0f, 1.0f, ImVec2(0, 80));
        // PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
        ImGui::PlotLines("RSSI",
                         m_Values.data(),
                         std::ssize(m_Values),
                         0,
                         std::format("Average: {}", rssi_avg()).c_str(),
                         -1.0f,
                         1.0f,
                         ImVec2{ -FLT_MIN, 100.0f },
                         sizeof(typename decltype(m_Values)::value_type));
        
        // PlotLines(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
        //ImGui::PlotLines("RSSI", [](void* data, int32_t index) -> float
        //{
        //    std::vector<float>* pValues = static_cast<decltype(m_Values)*>(data);
        //    int32_t i = index % std::ssize(*pValues);
        //    return (*pValues)[i];
        //}, m_Values, std::ssize(m_Values), );
    }
    
    ImGui::End();
}
void CRSSIPlot::add_rssi_value(float value)
{
    size_t index = m_Index % m_Values.size();
    m_Values[index] = value;
    ++m_Index;
}
float CRSSIPlot::rssi_avg() const
{
    float sum = std::accumulate(std::begin(m_Values), std::end(m_Values), 0.0f);
    
    return sum / static_cast<float>(m_Values.size());
}
}   // namespace gui