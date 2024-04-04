#pragma once
#include "CWidget.hpp"


namespace gui
{
class CGui
{
public:
	CGui();
	~CGui() = default;
	CGui(const CGui& other) = default;
	CGui(CGui&& other) = default;
	CGui& operator=(const CGui& other) = default;
	CGui& operator=(CGui&& other) = default;

    template<typename widget_t, typename... ctor_args_t>
    void emplace_widget(ctor_args_t&&... args)
    {
        m_Widgets.emplace(gui::make_widget<widget_t>(std::forward<ctor_args_t>(args)...));
    }
	void push();
private:
	std::vector<gui::CWidget> m_Widgets;
};
}	// namespace gui