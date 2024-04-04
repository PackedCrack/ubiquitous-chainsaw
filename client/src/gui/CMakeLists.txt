SET(GUI_SRCS
        CGui.cpp
        CGui.hpp
        CWidget.hpp
        CDeviceList.cpp
)


target_sources(${MAIN_PROJECT} PRIVATE ${GUI_SRCS})