SET(GFX_SRCS 
CWindow.cpp
CWindow.hpp
CRenderer.cpp
CRenderer.hpp
SDL_Defines.hpp
)

target_sources(${MAIN_PROJECT} PRIVATE ${GFX_SRCS})