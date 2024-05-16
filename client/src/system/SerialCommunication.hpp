#pragma once
#ifdef WIN32
    #include "windows/CSerialCommunication.hpp"
#else
    #error Only windows implemented atm
#endif
//
//
//
//
namespace sys
{
template<typename serial_t>
concept serial_communication = requires() {
    requires std::same_as<typename serial_t::make_type, std::expected<CSerialCommunication, ErrorSerialCom>>;
    { serial_t::make() } -> std::same_as<typename serial_t::make_type>;
};
template<typename serial_t = CSerialCommunication>
requires serial_communication<serial_t>
[[nodiscard]] serial_t::make_type open_serial_communication()
{
    return serial_t::make();
}
}    // namespace sys
