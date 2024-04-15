#pragma once
#include <string>
#include <stdexcept>


namespace exception
{
    class fatal_error : public std::runtime_error
    {
    public:
        explicit fatal_error(const char* msg)
                : runtime_error{ msg }
                , m_Message{ msg }
        {}
        [[nodiscard]] inline const char* what() const noexcept override
        {
            return m_Message.c_str();
        }
    private:
        std::string m_Message;
    };
}	// namespace exception