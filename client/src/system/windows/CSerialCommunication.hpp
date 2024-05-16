#pragma once
#include "../Errors.hpp"
//
//
//
//
namespace sys
{
class CSerialCommunication
{
public:
    using make_type = std::expected<CSerialCommunication, ErrorSerialCom>;
public:
    [[nodiscard]] static make_type make();
public:
    ~CSerialCommunication() = default;
    CSerialCommunication(const CSerialCommunication& other) = default;
    CSerialCommunication(CSerialCommunication&& other) = default;
    CSerialCommunication& operator=(const CSerialCommunication& other) = default;
    CSerialCommunication& operator=(CSerialCommunication&& other) = default;
private:
    explicit CSerialCommunication(std::string_view comPort);
public:
    // void write();
private:
    // HANDLE
};
}    // namespace sys
