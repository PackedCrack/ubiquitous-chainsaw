#pragma once
#include "CWidget.hpp"
#include "CDeviceList.hpp"
#include "CRSSIPlot.hpp"
namespace gui
{
using Widget = std::variant<CDeviceList, CRSSIPlot>;

template<typename widget_t>
concept imgui_renderable = requires(widget_t widget) {
    common::member_of_variant<widget_t, Widget>();

    { std::copy_constructible<widget_t> };
    { std::is_copy_assignable_v<widget_t> };
    { std::move_constructible<widget_t> };
    { std::is_move_assignable_v<widget_t> };
    { widget_t::KEY } -> std::convertible_to<decltype(widget_t::KEY)>;

    { widget.push() };
};
class CGui
{
    using KeyType = std::string_view;
public:
    explicit CGui(std::function<void()>&& generateKeyAction,
                  std::function<void(CRSSIPlot& rssiPlot, gui::CDeviceList& deviceList)>&& deleteKeyAction);
    ~CGui();
    CGui(const CGui& other) = default;
    CGui(CGui&& other) = default;
    CGui& operator=(const CGui& other) = default;
    CGui& operator=(CGui&& other) = default;
public:
    template<typename widget_t, typename... ctor_args_t>
    requires imgui_renderable<widget_t>
    [[nodiscard]] widget_t& emplace(ctor_args_t&&... args)
    {
        static_assert(std::same_as<std::remove_cvref_t<decltype(widget_t::KEY)>, KeyType>);
        // cppcheck-suppress redundantAssignment
        auto [iter, emplaced] = m_Widgets.try_emplace(widget_t::KEY, std::in_place_type<widget_t>, std::forward<ctor_args_t>(args)...);
        ASSERT(emplaced, "Adding widgets should never fail..");

        return std::get<widget_t>(iter->second);
    }
    template<typename widget_t>
    requires imgui_renderable<widget_t>
    [[nodiscard]] std::optional<std::reference_wrapper<widget_t>> find()
    {
        auto iter = m_Widgets.find(widget_t::KEY);
        return iter != std::end(m_Widgets) ? std::make_optional<std::reference_wrapper<widget_t>>(std::get<widget_t>(iter->second))
                                           : std::nullopt;
    }
    void push();
private:
    void push_dock_space();
    void push_menu_bar();
    void push_menu_keys();
    void push_all_widgets();
private:
    std::map<KeyType, Widget> m_Widgets;
    std::function<void()> m_GenerateKeysAction;
    std::function<void(CRSSIPlot&, CDeviceList&)> m_DeleteKeysAction;
};
}    // namespace gui
