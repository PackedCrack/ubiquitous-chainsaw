//
// Created by qwerty on 2024-04-10.
//
#pragma once
#include "../CServer.hpp"
#include "../common/CStopWatch.hpp"
#include "../CRssiDemander.hpp"
// clang-format off


// clang-format on
namespace gui
{
class CRSSIPlot
{
public:
    static constexpr std::string_view KEY = "rssiplot";
public:
    explicit CRSSIPlot(std::size_t size, CRssiDemander demander);
    ~CRSSIPlot() = default;
    CRSSIPlot(const CRSSIPlot& other) = delete;
    CRSSIPlot(CRSSIPlot&& other) = default;
    CRSSIPlot& operator=(const CRSSIPlot& other) = delete;
    CRSSIPlot& operator=(CRSSIPlot&& other) = default;
public:
    void push();
    [[nodiscard]] float rssi_avg() const;
private:
    void plot();
    void add_rssi_value(float value);
private:
    std::size_t m_Index;
    std::vector<float> m_Values;
    CRssiDemander m_Demander;
};
}    // namespace gui
