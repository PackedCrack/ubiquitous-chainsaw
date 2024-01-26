//
// Created by qwerty on 2024-01-26.
//
#pragma once
#include "windows/CScanner.hpp"

#include <memory>

namespace ble
{
template<typename implementation_t>
concept ScannerImplementation = requires(implementation_t impl)
{
    impl.scan();
};

class CBLEScanner
{
private:
    class IBase
    {
    public:
        virtual ~IBase() = default;
        [[nodiscard]] virtual std::unique_ptr<IBase> copy() const = 0;
    protected:
        IBase() = default;
        IBase(const IBase& other) = default;
        IBase(IBase&& other) = default;
        IBase& operator=(const IBase& other) = default;
        IBase& operator=(IBase&& other) = default;
    };
    template<typename platform_t>
    class Concrete : public IBase
    {
    public:
        explicit Concrete(platform_t&& protocol)
                : m_BLEScanner{ std::forward<platform_t>(protocol) }
        {}
        ~Concrete() override = default;
        Concrete(const Concrete& other) = default;
        Concrete(Concrete&& other) = default;
        Concrete& operator=(const Concrete& other) = default;
        Concrete& operator=(Concrete&& other) = default;
    public:
    
    private:
        platform_t m_BLEScanner;
    };
public:
    template<typename implementation_t>
    explicit CBLEScanner(implementation_t&& platformScanner) requires ScannerImplementation<implementation_t>
        : m_PlatformScanner{ std::make_unique<Concrete>(std::forward<platformScanner>(platformScanner)) }
    {}
    ~CBLEScanner() = default;
    CBLEScanner(const CBLEScanner& other)
        : m_PlatformScanner{ other.m_PlatformScanner->copy() }
    {}
    CBLEScanner(CBLEScanner&& other) = default;
    CBLEScanner& operator=(const CBLEScanner& other)
    {
        if(this != &other)
        {
            m_PlatformScanner = other.m_PlatformScanner->copy();
        }
        
        return *this;
    }
    CBLEScanner& operator=(CBLEScanner&& other) = default;
private:
    std::unique_ptr<IBase> m_PlatformScanner;
};

template<typename... ctor_args_t>
[[nodiscard]] CBLEScanner make_scanner(ctor_args_t&&... args)
{
    // TODO: ifdef for linux?
    return CBLEScanner{ win::CScanner{ /*std::forward<ctor_args_t>(args)...)*/} };
}
}   // namespace ble