//
// Created by hackerman on 1/23/24.
//

#pragma once
#include "CThreadSafeQueue.hpp"
template<typename job_t>
class CScheduler
{
private:
    static constexpr int64_t NO_JOBS = 0;
public:
    explicit CScheduler(std::function<void(job_t&&)> jobHandler)
        : m_JobHandler{ std::move(jobHandler) }
        , m_Queue{}
        , m_Peon{ [this]()
                  {
                      while (!m_StopSignal.load(std::memory_order_relaxed))
                      {
                          while (m_Jobs.load(std::memory_order_relaxed) > NO_JOBS)
                          {
                              if (m_Jobs.load(std::memory_order_relaxed) == NO_JOBS)
                              {
                                  m_Jobs.wait(NO_JOBS);
                              }

                              this->schedule_job();
                              [[maybe_unused]] int64_t result = m_Jobs.fetch_sub(1, std::memory_order_relaxed);
                          }
                      }
                  } }
        , m_Jobs{ NO_JOBS }
        , m_StopSignal{ false } {};
    ~CScheduler()
    {
        m_StopSignal.store(true, std::memory_order_relaxed);

        if (m_Peon.joinable())
        {
            m_Peon.join();
        }
    };
    CScheduler(const CScheduler& other) = default;
    CScheduler(CScheduler&& other) = default;
    CScheduler& operator=(const CScheduler& other) = default;
    CScheduler& operator=(CScheduler&& other) = default;
    void schedule_job()
    {
        job_t job{};
        m_Queue.pop(&job);

        if (m_JobHandler)
        {
            m_JobHandler(std::move(job));
        }
        else
        {
            //LOG_ERROR("Schedulers job handler is empty!");
        }
    };
    template<typename value_t>
    void push_job(value_t&& data)
    {
        m_Queue.push(std::forward<value_t>(data));

        [[maybe_unused]] int64_t result = m_Jobs.fetch_add(1, std::memory_order_relaxed);
        m_Jobs.notify_one();
    }
    template<typename... ctor_args_t>
    void emplace_job(ctor_args_t&&... args)
    {
        m_Queue.emplace(std::forward<ctor_args_t>(args)...);

        [[maybe_unused]] int64_t result = m_Jobs.fetch_add(1, std::memory_order_relaxed);
        m_Jobs.notify_one();
    }
private:
    std::function<void(job_t)> m_JobHandler;
    CThreadSafeQueue<job_t> m_Queue;
    std::thread m_Peon;
    std::atomic<int64_t> m_Jobs;
    std::atomic<bool> m_StopSignal;
};
