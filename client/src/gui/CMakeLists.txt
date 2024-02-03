SET(GUI_SRCS 
    CGui.cpp
    CGui.hpp
    CWidget.hpp
)


target_sources(${MAIN_PROJECT} PRIVATE ${GUI_SRCS})