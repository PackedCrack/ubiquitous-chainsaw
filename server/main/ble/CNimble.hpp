#pragma once
// project
#include "CGap.hpp"
#include "profiles/CProfileCache.hpp"
// std
#include <memory>
#include <mutex>
#include <condition_variable>
// nimble
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"
#include "host/ble_hs.h"
namespace ble
{
class CNimble
{
public:
    CNimble();
    ~CNimble();
    CNimble(const CNimble& other) = delete;
    CNimble(CNimble&& other) = default;
    CNimble& operator=(const CNimble& other) = delete;
    CNimble& operator=(CNimble&& other) = default;
public:
    static void sync_callback();
private:
    [[nodiscard]] static std::pair<std::shared_ptr<std::mutex>, std::shared_ptr<std::condition_variable>> synchronization_primitives();
private:
    std::unique_ptr<CGap> m_pGap = nullptr;
    std::unique_ptr<CProfileCache> m_pProfileCache = nullptr;
};
}    // namespace ble
