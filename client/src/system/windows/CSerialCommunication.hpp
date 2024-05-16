#pragma once
#include "../Errors.hpp"
#include "common/common.hpp"
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
    ~CSerialCommunication();
    CSerialCommunication(const CSerialCommunication& other) = delete;
    CSerialCommunication(CSerialCommunication&& other) noexcept;
    CSerialCommunication& operator=(const CSerialCommunication& other) = delete;
    CSerialCommunication& operator=(CSerialCommunication&& other) noexcept;
private:
    explicit CSerialCommunication(std::string_view comPort);
public:
    // void write();
    template<typename buffer_t>
    requires common::buffer<buffer_t>
    [[nodiscard]] uint32_t write(buffer_t&& buffer) const
    {
        DWORD bytesWritten{};
        // https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
        if (!WriteFile(m_Handle, buffer.data(), common::assert_down_cast<DWORD>(buffer.size()), &bytesWritten, nullptr))
        {
            /*The WriteFile function may fail with ERROR_INVALID_USER_BUFFER or 
            ERROR_NOT_ENOUGH_MEMORY whenever there are too many outstanding asynchronous I/O requests.*/
            LOG_ERROR_FMT("Failed to write data over serial. Reason: \"{}\"", CErrorMessage{ GetLastError() }.message());
        }

        return bytesWritten;
    }
private:
    void set_serial_timeouts() const;
    void set_serial_settings() const;
    [[nodiscard]] DCB serial_settings() const;
private:
    HANDLE m_Handle = INVALID_HANDLE_VALUE;
};
}    // namespace sys
