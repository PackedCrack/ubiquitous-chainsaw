#pragma once


namespace common
{
template<typename clock_t>
concept chrono_clock = std::is_same_v<clock_t, std::chrono::steady_clock> ||
                      std::is_same_v<clock_t, std::chrono::system_clock> ||
                      std::is_same_v<clock_t, std::chrono::high_resolution_clock>;

template<typename clock_t>
concept chrono_duration = std::is_same_v<clock_t, std::chrono::nanoseconds> ||
                      std::is_same_v<clock_t, std::chrono::microseconds> ||
                      std::is_same_v<clock_t, std::chrono::milliseconds> ||
                      std::is_same_v<clock_t, std::chrono::seconds> ||
                      std::is_same_v<clock_t, std::chrono::minutes> ||
                      std::is_same_v<clock_t, std::chrono::hours> ||
                      std::is_same_v<clock_t, std::chrono::days> ||
                      std::is_same_v<clock_t, std::chrono::weeks> ||
                      std::is_same_v<clock_t, std::chrono::months> ||
                      std::is_same_v<clock_t, std::chrono::years>;


template<typename duration_t = std::chrono::milliseconds, typename clock_t = std::chrono::steady_clock>
requires chrono_clock<clock_t> && chrono_duration<duration_t>
class CStopWatch
{
public:
    CStopWatch()
            : m_Timepoint{ clock_t::now() }
    {}
	~CStopWatch() = default;
    CStopWatch(const CStopWatch& other) = default;
    CStopWatch(CStopWatch&& other) = default;
    CStopWatch& operator=(const CStopWatch& other) = default;
    CStopWatch& operator=(CStopWatch&& other) = default;
public:
    void reset() { m_Timepoint = clock_t::now(); };
    
    template<typename real_t>
    requires std::is_floating_point_v<real_t>
    [[nodiscard]] real_t lap()
    {
        std::chrono::time_point<clock_t> end = clock_t::now();
        return std::chrono::duration<real_t, typename duration_t::period>(end - m_Timepoint).count();
    }
    template<typename real_t>
    requires std::is_floating_point_v<real_t>
    [[nodiscard]] real_t time_elapsed()
    {
        auto elapsed = lap<real_t>();
        reset();
        return elapsed;
    }
private:
    std::chrono::time_point<clock_t> m_Timepoint;
};
}	// namespace common