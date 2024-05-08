//
// Created by qwerty on 2024-05-04.
//
#include "CReplayProtector.hpp"
#include "client_defines.hpp"
//
//
//
//
namespace
{
[[nodiscard]] std::unique_ptr<security::CRandom> make_rng()
{
    std::expected<security::CRandom, security::CRandom::Error> expected = security::CRandom::make_rng();
    ASSERT(expected.has_value(), "Failed to create cryptographic rng generator");

    return std::make_unique<security::CRandom>(std::move(*expected));
}
}    // namespace
CReplayProtector::CReplayProtector(std::chrono::seconds storeDuration)
    : m_PacketCache{}
    , m_PacketAges{}
    , m_pRng{ make_rng() }
    , m_Generator{ std::random_device{}() }
    , m_OldestAllowed{ storeDuration }
{}
CReplayProtector::CReplayProtector(const CReplayProtector& other)
    : m_PacketCache{}
    , m_PacketAges{}
    , m_pRng{ make_rng() }
    , m_Generator{ other.m_Generator }
    , m_OldestAllowed{ other.m_OldestAllowed }
{}
CReplayProtector& CReplayProtector::operator=(const CReplayProtector& other)
{
    if (this != &other)
    {
        m_PacketCache = other.m_PacketCache;
        m_PacketAges = other.m_PacketAges;
        m_pRng = make_rng();
        m_Generator = other.m_Generator;
        m_OldestAllowed = other.m_OldestAllowed;
    }

    ASSERT(m_pRng != nullptr, "Expected Rng to be initialized.");

    return *this;
}
bool CReplayProtector::expected_random_data(std::span<const uint8_t> randomData)
{
    remove_outdated_packets();

    Packet key{ .randomData = randomData };
    auto iter = m_PacketCache.find(key);
    if (iter == std::end(m_PacketCache))
    {
        return false;
    }
    if (iter->beenAnswered)
    {
        return false;
    }

    auto node = m_PacketCache.extract(iter);
    node.value().beenAnswered = true;
    m_PacketCache.insert(std::move(node));

    return true;
}
const std::vector<byte>& CReplayProtector::generate_random_block()
{
    static std::uniform_int_distribution<size_t> distribution{ 64u, 96u };

    size_t blockSize = distribution(m_Generator);
    std::expected<std::vector<byte>, security::CRandom::Error> expected = m_pRng->generate_block(blockSize);
    ASSERT(expected.has_value(), "Generating random data should never fail");

    auto [iter, emplaced] = m_PacketCache.emplace(Packet{ .randomData = std::move(*expected) });
    ASSERT(emplaced == true, "Expected a unique sequence of bytes");

    m_PacketAges.push(PacketAge{ .packet = iter, .timer{} });

    const std::vector<uint8_t>* pRandomData = std::get_if<std::vector<uint8_t>>(&(iter->randomData));
    ASSERT(pRandomData != nullptr, "Failed to access variant which should always be stored as std::vector<uint8_t> in the cache");

    return *pRandomData;
}
void CReplayProtector::remove_outdated_packets()
{
    while (!m_PacketAges.empty())
    {
        const PacketAge& oldestPacket = m_PacketAges.top();
        if (oldestPacket.timer.lap<float>() >= static_cast<float>(m_OldestAllowed.count()))
        {
            m_PacketCache.erase(oldestPacket.packet);
            m_PacketAges.pop();
        }
        else
        {
            break;
        }
    }
}
