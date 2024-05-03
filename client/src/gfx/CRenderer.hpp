#pragma once
#include "CWindow.hpp"
//third_party
#include "SDL3/SDL_render.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl3.h"
#include "imgui/imgui_impl_sdlrenderer3.h"
namespace gfx
{
class CRenderer
{
public:
    CRenderer(CWindow& window, uint32_t flags);
    ~CRenderer();
    CRenderer(const CRenderer& other) = delete;
    CRenderer(CRenderer&& other) noexcept;
    CRenderer& operator=(const CRenderer& other) = delete;
    CRenderer& operator=(CRenderer&& other) noexcept;

    void begin_frame() const;
    void end_frame() const;
    void set_clear_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
    [[nodiscard]] SDL_Renderer* handle() const;
private:
    SDL_Renderer* m_pRenderer = nullptr;
    ImGuiContext* m_pContext = nullptr;
    ImGuiIO* m_pGuiIO = nullptr;
};
}    // namespace gfx
