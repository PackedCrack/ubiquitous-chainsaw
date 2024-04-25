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
    CGui();
    ~CGui();
    CGui(const CGui& other) = default;
    CGui(CGui&& other) = default;
    CGui& operator=(const CGui& other) = default;
    CGui& operator=(CGui&& other) = default;
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
    void push();
private:
    //std::vector<gui::CWidget> m_Widgets;
    std::map<KeyType, Widget> m_Widgets;
};
}    // namespace gui
