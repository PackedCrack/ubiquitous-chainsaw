//
// Created by qwerty on 2024-04-04.
//

#pragma once
#include <utility>
template<typename T>
class Pointer
{
public:
    Pointer() = default;
    explicit Pointer(T* p)
        : m_Pointer{ p }
    {}
    // This cannot be explicit if we want to be able to do: Pointer = nullptr;
    // cppcheck-suppress noExplicitConstructor
    Pointer(std::nullptr_t null)
        : m_Pointer{ null }
    {}
    ~Pointer() = default;
    Pointer(const Pointer& other)
        : m_Pointer{ other.m_Pointer }
    {}
    Pointer(Pointer&& other) noexcept
        : m_Pointer{ std::exchange(other.m_Pointer, nullptr) }
    {}
    Pointer& operator=(const Pointer& other)
    {
        m_Pointer = other.m_Pointer;
        return *this;
    }
    Pointer& operator=(Pointer&& other) noexcept
    {
        m_Pointer = std::exchange(other.m_Pointer, nullptr);
        return *this;
    }
    Pointer& operator=(std::nullptr_t null)
    {
        m_Pointer = null;
        return *this;
    }
    template<typename ptr_t>
    requires std::is_pointer_v<ptr_t>
    Pointer& operator=(ptr_t&& newPtr)
    {
        m_Pointer = newPtr;
        return *this;
    }
    auto& operator*() const { return *m_Pointer; }
    T* operator->() const { return m_Pointer; }
private:
    T* m_Pointer = nullptr;
};
