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

	void push();
private:
	std::vector<gui::CWidget> m_Widgets;
};
}	// namespace gui