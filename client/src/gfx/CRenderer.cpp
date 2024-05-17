#include "CRenderer.hpp"
#include "SDL_Defines.hpp"
namespace
{
ImGuiContext* create_imgui_context()
{
    IMGUI_CHECKVERSION();
    ImGuiContext* pContext = ImGui::CreateContext();
    ASSERT(pContext, "ImGui Context is nullptr!");

    return pContext;
}
ImGuiIO* create_imgui_io()
{
    ImGuiIO* pGuiIO = &ImGui::GetIO();
    ASSERT(pGuiIO, "ImGuiIO is nullptr!");

    return pGuiIO;
}
}    // namespace
namespace gfx
{
CRenderer::CRenderer(CWindow& window)
    : m_pRenderer{ SDL_CreateRenderer(window.handle(), nullptr) }
    , m_pContext{ create_imgui_context() }
    , m_pGuiIO{ create_imgui_io() }
{
    ASSERT(m_pRenderer != nullptr, "Failed to create renderer!");

    bool success = ImGui_ImplSDL3_InitForSDLRenderer(window.handle(), m_pRenderer);
    if (!success)
    {
        LOG_FATAL("Failed to init imgui sdl3 implemenatation!");
    }

    success = ImGui_ImplSDLRenderer3_Init(m_pRenderer);
    if (!success)
    {
        LOG_FATAL("Failed to init imgui renderer implemenatation!");
    }


    m_pGuiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}
CRenderer::~CRenderer()
{
    if (!m_pRenderer)
    {
        return;
    }

    SDL_DestroyRenderer(m_pRenderer);
}
CRenderer::CRenderer(CRenderer&& other) noexcept
{
    std::swap(m_pRenderer, other.m_pRenderer);
    std::swap(m_pContext, other.m_pContext);
    std::swap(m_pGuiIO, other.m_pGuiIO);
}
CRenderer& CRenderer::operator=(CRenderer&& other) noexcept
{
    std::swap(m_pRenderer, other.m_pRenderer);
    std::swap(m_pContext, other.m_pContext);
    std::swap(m_pGuiIO, other.m_pGuiIO);

    return *this;
}
void CRenderer::begin_frame() const
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}
void CRenderer::end_frame() const
{
    SDL_CHECK(SDL_RenderClear(m_pRenderer), "Failed to clear backbuffer!");

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_pRenderer);

    SDL_CHECK(SDL_RenderPresent(m_pRenderer), "Failed to present buffer");
}
void CRenderer::set_clear_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
    ASSERT(m_pRenderer, "Renderer is nullptr!");

    SDL_CHECK(SDL_SetRenderDrawColor(m_pRenderer, red, green, blue, alpha), "Failed to set clear color!");
}
SDL_Renderer* CRenderer::handle() const
{
    ASSERT(m_pRenderer, "Renderer is nullptr!");

    return m_pRenderer;
}
}    // namespace gfx
