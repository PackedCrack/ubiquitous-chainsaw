#include "CConnection.hpp"
// std
#include <utility>


namespace ble
{
CConnection::CConnection(ConnectionHandle handle)	
	: m_Handle{ std::move(handle) }
{}
CConnection::~CConnection()
{
	if(m_Handle == INVALID_HANDLE)
		return;
	
	
	m_Handle = INVALID_HANDLE;
	std::optional<Error> result = drop(DropCode::completed);
	if(result)
	{
		if(result->code == NimbleErrorCode::noConnection)
		{
			LOG_WARN_FMT("{}", result->msg);
		}
		else if(result->code == NimbleErrorCode::unexpectedFailure)
		{
			LOG_ERROR_FMT("{}", result->msg);
		}
	}
}
CConnection::CConnection(CConnection&& other) noexcept
	: m_Handle{ m_Handle = std::exchange(other.m_Handle, INVALID_HANDLE) }
{}
CConnection& CConnection::operator=(CConnection&& other) noexcept
{
	if(this != &other)
	{
		m_Handle = std::exchange(other.m_Handle, INVALID_HANDLE);
	}

	return *this;
}
/// @brief 
/// @param CConection::DropCode reason 
/// @return std::nullopt on success. 
/// NimbleErrorCode::noConnection if there is no connection with the specified handle. 
/// NimbleErrorCode::unknown on failure.
std::optional<CConnection::Error> CConnection::drop(DropCode reason)
{
	int32_t result = ble_gap_terminate(m_Handle, static_cast<int32_t>(reason));
    if(result == static_cast<int32_t>(NimbleErrorCode::success))
		return std::nullopt;


	if(result == static_cast<int32_t>(NimbleErrorCode::noConnection))
	{
		return std::make_optional<Error>(NimbleErrorCode::noConnection, "Tried to drop non existing connection.");
	}
	else
	{
		return std::make_optional<Error>(
			NimbleErrorCode::noConnection, FMT("Unknown error recieved.. Return code from nimble: \"{}\"", result));
	}
}
ConnectionHandle CConnection::handle() { return m_Handle; }
}	// namespace ble