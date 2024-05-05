//
// Created by qwerty on 2024-05-04.
//
#pragma once
#include "common/CStopWatch.hpp"
//
//
//
//
class CReplayProtector
{
private:
    struct Packet
    {
        std::vector<uint8_t> randomData;
        common::CStopWatch<std::chrono::seconds> timer;
    };
    struct PacketAge
    {
        std::unordered_set<Packet>::const_iterator packet;
        [[nodiscard]] friend inline bool operator<(const PacketAge& lhs, const PacketAge& rhs)
        {
            return lhs.packet->timer.lap<float>() < rhs.packet->timer.lap<float>();
        }
    };
public:
    CReplayProtector() = default;
    ~CReplayProtector() = default;
    CReplayProtector(const CReplayProtector& other) = default;
    CReplayProtector(CReplayProtector&& other) = default;
    CReplayProtector& operator=(const CReplayProtector& other) = default;
    CReplayProtector& operator=(CReplayProtector&& other) = default;
public:
    [[nodiscard]] bool expecting_packet(std::span<uint8_t> packet);
private:
    std::unordered_set<Packet> m_PacketCache;
    std::priority_queue<PacketAge> m_PacketAges;
};
