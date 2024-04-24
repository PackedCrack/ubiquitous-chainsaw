#include "CConnection.hpp"
// std
#include <utility>
// ble
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wshadow"
#include "host/../../src/ble_hs_conn_priv.h"
#pragma GCC diagnostic pop


extern ble_hs_conn* ble_hs_conn_find(uint16_t conn_handle);
namespace ble
{
CConnection::CConnection(ConnectionHandle handle)
    : m_Handle{ handle }
{}
CConnection::~CConnection()
{
    if (!m_Handle)
    {
        return;
    }

    if (ble_hs_conn_find(*m_Handle) == nullptr)
    {
        return;
    }

    std::optional<Error> error = drop(DropCode::completed);
    if (error)
    {
        if (error->code == NimbleErrorCode::noConnection)
        {
            LOG_WARN_FMT("{}", error->msg);
        }
        else if (error->code == NimbleErrorCode::unexpectedFailure)
        {
            LOG_ERROR_FMT("{}", error->msg);
        }
    }
    else
    {
        m_Handle = std::nullopt;
    }
}
CConnection::CConnection(CConnection&& other) noexcept
    : m_Handle{ std::exchange(other.m_Handle, std::nullopt) }
{}
CConnection& CConnection::operator=(CConnection&& other) noexcept
{
    if (this != &other)
    {
        m_Handle = std::exchange(other.m_Handle, std::nullopt);
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
    int32_t result = ble_gap_terminate(*m_Handle, static_cast<int32_t>(reason));
    if (result == static_cast<int32_t>(NimbleErrorCode::success))
    {
        return std::nullopt;
    }


    if (result == static_cast<int32_t>(NimbleErrorCode::noConnection))
    {
        return std::make_optional<Error>(NimbleErrorCode::noConnection, "Tried to drop non existing connection.");
    }
    else
    {
        return std::make_optional<Error>(NimbleErrorCode::noConnection,
                                         FMT("Unknown error recieved.. Return code from nimble: \"{}\"", result));
    }
}
std::optional<ConnectionHandle> CConnection::handle() const
{
    return m_Handle;
}
}    // namespace ble
