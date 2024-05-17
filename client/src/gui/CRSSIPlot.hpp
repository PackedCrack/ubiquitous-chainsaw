//
// Created by qwerty on 2024-04-10.
//
#pragma once
#include "../CServer.hpp"
#include "../common/CStopWatch.hpp"
#include "../CRssiDemander.hpp"
//
//
//
//
namespace gui
{
class CRSSIPlot
{
public:
    static constexpr std::string_view KEY = "rssiplot";
public:
    explicit CRSSIPlot(std::shared_ptr<CRssiDemander> pDemander, std::size_t size = 14);
    ~CRSSIPlot() = default;
    CRSSIPlot(const CRSSIPlot& other) = delete;
    CRSSIPlot(CRSSIPlot&& other) = default;
    CRSSIPlot& operator=(const CRSSIPlot& other) = delete;
    CRSSIPlot& operator=(CRSSIPlot&& other) = default;
public:
    void push();
    [[nodiscard]] int8_t rssi_median() const;
private:
    void plot();
    void add_rssi_value(int8_t value);
private:
    std::size_t m_Index;
    std::size_t m_MaxSize;
    std::vector<float> m_Values;
    std::shared_ptr<CRssiDemander> m_pDemander;
};
}    // namespace gui
