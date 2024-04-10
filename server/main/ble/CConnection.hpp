#pragma once
#include "ble_common.hpp"
// std
#include <optional>


namespace ble
{
struct ConnectionHandle
{
	template<typename integer_t> 
	requires std::integral<integer_t>
	constexpr ConnectionHandle(integer_t id)
		: m_ID{ static_cast<uint16_t>(id) }
	{
		ASSERT(id <= UINT16_MAX, "Conversion wrap around!");
	}
	template<typename integer_t> 
	requires std::integral<integer_t>
	constexpr ConnectionHandle& operator=(integer_t id)
	{
		ASSERT(id <= UINT16_MAX, "Conversion wrap around!");
		m_ID = static_cast<uint16_t>(id);

		return *this;
	}
	inline operator uint16_t() { return m_ID; }
	[[nodiscard]] friend bool operator==(ConnectionHandle lhs, ConnectionHandle rhs) { return lhs.m_ID == rhs.m_ID; }
	[[nodiscard]] friend bool operator!=(ConnectionHandle lhs, ConnectionHandle rhs) { return lhs.m_ID == rhs.m_ID; }
private:
	uint16_t m_ID;
};
class CConnection
{
	using InvalidHandle = std::nullopt_t;
public:
	struct Error
	{
		NimbleErrorCode code;
		std::string msg;
	};
	enum class DropCode : int32_t
	{
		completed = static_cast<int32_t>(NimbleErrorCode::operationCompleted),
		timeOut = static_cast<int32_t>(NimbleErrorCode::operationTimeOut),
		busy = static_cast<int32_t>(NimbleErrorCode::isBusy),
		resourceExhaustion = static_cast<int32_t>(NimbleErrorCode::resourceExhaustion),
		unexpectedCallbackBehavior = static_cast<int32_t>(NimbleErrorCode::unexpectedCallbackBehavior),
		unexpectedError = static_cast<int32_t>(NimbleErrorCode::unexpectedFailure)
	};
	[[nodiscard]] inline operator bool() { return m_Handle != std::nullopt; }
	[[nodiscard]] inline friend bool operator==(const CConnection& lhs, const CConnection& rhs) { return lhs.m_Handle == rhs.m_Handle; }
	[[nodiscard]] inline friend bool operator!=(const CConnection& lhs, const CConnection& rhs) { return lhs.m_Handle == rhs.m_Handle; }
public:
	CConnection() = default;;
	CConnection(ConnectionHandle handle);
	~CConnection();
	CConnection(const CConnection& other) = delete;
	CConnection(CConnection&& other) noexcept;
	CConnection& operator=(const CConnection& other) = delete;
	CConnection& operator=(CConnection&& other) noexcept;
public:
	[[nodiscard]] std::optional<Error> drop(DropCode reason);
	[[nodiscard]] std::optional<ConnectionHandle> handle() const;
private:
	std::optional<ConnectionHandle> m_Handle = std::nullopt;
};
}	// namespace ble