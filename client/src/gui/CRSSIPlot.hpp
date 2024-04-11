//
// Created by qwerty on 2024-04-10.
//

#pragma once


namespace gui
{
class CRSSIPlot
{
public:
    static constexpr std::string_view KEY = "rssiplot";
public:
    explicit CRSSIPlot(size_t size);
    ~CRSSIPlot() = default;
    CRSSIPlot(const CRSSIPlot& other) = default;
    CRSSIPlot(CRSSIPlot&& other) = default;
    CRSSIPlot& operator=(const CRSSIPlot& other) = default;
    CRSSIPlot& operator=(CRSSIPlot&& other) = default;
public:
    void push() const;
    void add_rssi_value(float value);
private:
    [[nodiscard]] float rssi_avg() const;
private:
    size_t m_Index;
    std::vector<float> m_Values;
};
}   // namespace gui