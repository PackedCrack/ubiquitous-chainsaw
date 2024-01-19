#pragma once


namespace gui
{
class CWidget
{
private:
    friend void push_widget(CWidget& widget)
    {
        widget.m_pWidget->push();
    }

    class IBase
    {
    public:
        virtual ~IBase() = default;

        virtual void push() = 0;
        virtual std::unique_ptr<IBase> copy() const = 0;
    protected:
        IBase() = default;
        IBase(const IBase& other) = default;
        IBase(IBase&& other) = default;
        IBase& operator=(const IBase& other) = default;
        IBase& operator=(IBase&& other) = default;
    };
    template<typename gui_t>
    class Concrete : public IBase
    {
    public:
        explicit Concrete(gui_t&& gui)
            : m_Gui{ std::forward<gui_t>(gui) }
        {}
        ~Concrete() = default;
        Concrete(const Concrete& other) = default;
        Concrete(Concrete&& other) = default;
        Concrete& operator=(const Concrete& other) = default;
        Concrete& operator=(Concrete&& other) = default;

        void push() override
        {
            m_Gui.push();
        }
        std::unique_ptr<IBase> copy() const override
        {
            return std::make_unique<Concrete>(*this);
        }
    private:
        gui_t m_Gui;
    };
public:
    CWidget() = default;
    template<typename widget_t>
    explicit CWidget(widget_t&& gui)
        : m_pWidget{ std::make_unique<Concrete<widget_t>>(std::forward<widget_t>(gui)) }
    {}
    ~CWidget() = default;
    //CWidget(const CWidget& other) = delete;
    CWidget(const CWidget& other)
        : m_pWidget{ other.m_pWidget->copy() }
    {};
    CWidget(CWidget&& other) = default;
   // CWidget& operator=(const CWidget& other) = delete;
    CWidget& operator=(const CWidget& other)
    {
        m_pWidget = other.m_pWidget->copy();

        return *this;
    };
    CWidget& operator=(CWidget&& other) = default;

private:
    std::unique_ptr<IBase> m_pWidget;
};

template<typename gui_t>
concept imgui_widget = requires (gui_t widget)
{
    widget.push();
};

template<typename widget_t, typename... args>
[[nodiscard]] CWidget make_widget(args&&... constrArgs) requires imgui_widget<widget_t>
{
    return CWidget{ widget_t{ std::forward<args>(constrArgs)... } };
};
}	// namespace gui