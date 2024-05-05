//
// Created by qwerty on 2024-05-04.
//
#include "CReplayProtector.hpp"
//
//
//
//
//bool CReplayProtector::expecting_packet(std::span<uint8_t> packet)
//{
//    m_PacketCache.find(Packet{ .randomData = packet });
//
//    while (!m_PacketAges.empty())
//    {
//        static constexpr float OLDEST_ALLOWED = static_cast<float>(std::chrono::seconds(3).count());
//
//        std::unordered_set<Packet>::const_iterator oldestPacket = m_PacketAges.top().packet;
//        if (oldestPacket->timer.lap<float>() >= OLDEST_ALLOWED)
//        {
//            m_PacketCache.erase(oldestPacket);
//            m_PacketAges.pop();
//        }
//        else
//        {
//            break;
//        }
//    }
//}
