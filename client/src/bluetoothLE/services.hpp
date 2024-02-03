#pragma once


namespace ble
{

struct UUID
{
    uint32_t data1 = 0u;
    uint16_t custom = 0u;
    uint16_t data3 = 0u;
    uint16_t data4 = 0u;
    uint16_t data5 = 0u;
    uint16_t data6 = 0u;
    uint16_t data7 = 0u;
};

static constexpr UUID BaseUID {
        .data1 = 0u,
        .data2 = 0u,
        .data3 = 0x1000,
        .data4 = 0x8000,
        .data5 = 0x0080,
        .data6 = 0x5F9B,
        .data7 = 0x34FB
    };

static constexpr uint16_t ID_SERVICE_WHOAMI = 0xAAAA;
}   // namespace ble