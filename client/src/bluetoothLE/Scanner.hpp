//
// Created by qwerty on 2024-01-26.
//
#pragma once
#ifdef WIN32
    #include "windows/CScanner.hpp"
#else
    #error Only windows is implemented atm
#endif
#include "common/common.hpp"
namespace ble
{
template<typename scanner_t, typename... ctor_args_t>
concept scanner = requires(scanner_t scanner, const scanner_t constScanner, scanner_t::pos_t pos) {
    typename scanner_t::pos_t;
    common::constructible_with<scanner_t, ctor_args_t...>;
    { scanner.begin_scan() };
    { constScanner.end_scan() };
    { constScanner.retrieve_n_devices(pos, pos) } -> std::convertible_to<std::vector<ble::DeviceInfo>>;
    { constScanner.num_devices() } -> std::convertible_to<const std::atomic<size_t>&>;
    { constScanner.scanning() } -> std::convertible_to<bool>;
};
template<typename scanner_t, typename... ctor_args_t>
requires scanner<scanner_t, ctor_args_t...>
[[nodiscard]] scanner_t make_scanner(ctor_args_t&&... args)
{
    return scanner_t{ std::forward<ctor_args_t>(args)... };
}
}    // namespace ble
//namespace ble
//{
//template<typename implementation_t>
//concept Scanner = requires(implementation_t impl)
//{
//    { impl.begin_scan() };
//    { impl.end_scan() };
//    { impl.found_devices() } -> std::convertible_to<std::vector<DeviceInfo>>;
//};
//
//class CBLEScanner
//{
//private:
//    friend void begin_scan(const CBLEScanner& scanner) { scanner.m_PlatformScanner->execute_begin_scan(); }
//    friend void end_scan(const CBLEScanner& scanner) { scanner.m_PlatformScanner->execute_end_scan(); }
//    [[nodiscard]] friend std::vector<DeviceInfo> found_devices(CBLEScanner& scanner)
//    { return scanner.m_PlatformScanner->execute_found_devices(); }
//    class IBase
//    {
//    public:
//        virtual ~IBase() = default;
//        IBase(const IBase& other) = delete;
//        IBase& operator=(const IBase& other) = delete;
//
//        virtual void execute_begin_scan() const = 0;
//        virtual void execute_end_scan() const = 0;
//        virtual std::vector<DeviceInfo> execute_found_devices() = 0;
//    protected:
//        IBase() = default;
//        IBase(IBase&& other) = default;
//        IBase& operator=(IBase&& other) = default;
//    };
//    template<typename scanner_t>
//    class Concrete final : public IBase
//    {
//    public:
//        explicit Concrete(scanner_t&& protocol)
//                : m_Scanner{ std::forward<scanner_t>(protocol) }
//        {}
//        ~Concrete() override = default;
//        Concrete(const Concrete& other) = delete;
//        Concrete(Concrete&& other) = default;
//        Concrete& operator=(const Concrete& other) = delete;
//        Concrete& operator=(Concrete&& other) = default;
//    public:
//        void execute_begin_scan() const override { m_Scanner.begin_scan(); }
//        void execute_end_scan() const override { m_Scanner.end_scan(); }
//        [[nodiscard]] std::vector<DeviceInfo> execute_found_devices() override { return m_Scanner.found_devices(); }
//    private:
//        scanner_t m_Scanner;
//    };
//public:
//    template<typename platform_scanner_t>
//    explicit CBLEScanner(platform_scanner_t&& platformScanner) requires Scanner<platform_scanner_t>
//        : m_PlatformScanner{ std::make_unique<Concrete<platform_scanner_t>>(std::forward<platform_scanner_t>(platformScanner)) }
//    {}
//    ~CBLEScanner() = default;
//    CBLEScanner(const CBLEScanner& other) = delete;
//    CBLEScanner(CBLEScanner&& other) = default;
//    CBLEScanner& operator=(const CBLEScanner& other) = delete;
//    CBLEScanner& operator=(CBLEScanner&& other) = default;
//
//    void begin_scan() const
//    {
//        m_PlatformScanner->execute_begin_scan();
//    }
//    void end_scan() const
//    {
//        m_PlatformScanner->execute_end_scan();
//    }
//    [[nodiscard]] std::vector<DeviceInfo> found_devices()
//    {
//        return m_PlatformScanner->execute_found_devices();
//    }
//private:
//    std::unique_ptr<IBase> m_PlatformScanner;
//};
//
//template<typename... ctor_args_t>
//[[nodiscard]] CBLEScanner make_scanner(ctor_args_t&&... args)
//{
//    // TODO: ifdef for linux?
//    return CBLEScanner{ CScanner{ std::forward<ctor_args_t>(args)... }};
//}
//}   // namespace ble
