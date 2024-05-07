//
// Created by qwerty on 2024-05-04.
//
#pragma once
#include "common/CStopWatch.hpp"
#include "security/CRandom.hpp"
//
//
//
//
class CReplayProtector
{
private:
    struct Packet
    {
        std::variant<std::vector<uint8_t>, std::span<uint8_t>> randomData;
        bool beenAnswered = false;
        [[nodiscard]] friend inline bool operator==(const Packet& lhs, const Packet& rhs)
        {
            bool equal{ true };
            std::visit(
                [&equal](auto&& lhs, auto&& rhs)
                {
                    if (lhs.size() == rhs.size())
                    {
                        for (std::size_t i = 0u; i < lhs.size(); ++i)
                        {
                            if (lhs[i] != rhs[i])
                            {
                                equal = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        equal = false;
                    }
                },
                lhs.randomData,
                rhs.randomData);

            return equal;
        }
        [[nodiscard]] friend inline bool operator!=(const Packet& lhs, const Packet& rhs)
        {
            bool equal{ false };
            std::visit(
                [&equal](auto&& lhs, auto&& rhs)
                {
                    if (lhs.size() == rhs.size())
                    {
                        for (std::size_t i = 0u; i < lhs.size(); ++i)
                        {
                            if (lhs[i] != rhs[i])
                            {
                                equal = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        equal = true;
                    }
                },
                lhs.randomData,
                rhs.randomData);

            return equal;
        }
    };
    struct PacketHasher
    {
        [[nodiscard]] inline std::size_t operator()(const Packet& packet) const
        {
            std::size_t hash{};
            std::visit(
                [&hash](auto&& data)
                {
                    for (auto byte : data)
                    {
                        hash ^= std::hash<uint8_t>{}(byte) * 31 + (hash << 7);
                    }
                },
                packet.randomData);

            return hash;
        }
    };
    struct PacketAge
    {
        std::unordered_set<Packet>::const_iterator packet;
        common::CStopWatch<std::chrono::seconds> timer;
        [[nodiscard]] friend inline bool operator<(const PacketAge& lhs, const PacketAge& rhs)
        {
            return lhs.timer.lap<float>() < rhs.timer.lap<float>();
        }
    };
public:
    CReplayProtector();
    ~CReplayProtector() = default;
    CReplayProtector(const CReplayProtector& other);
    CReplayProtector(CReplayProtector&& other) = default;
    CReplayProtector& operator=(const CReplayProtector& other);
    CReplayProtector& operator=(CReplayProtector&& other) = default;
public:
    [[nodiscard]] bool expected_packet(std::span<uint8_t> packet);
    [[nodiscard]] const std::vector<byte>& generate_random_block();
private:
    void remove_outdated_packets();
private:
    std::unordered_set<Packet, PacketHasher> m_PacketCache;
    std::priority_queue<PacketAge> m_PacketAges;
    std::unique_ptr<security::CRandom> m_pRng;
    std::mt19937_64 m_Generator;
};
