SET(GUI_SRCS
        CGui.cpp
        CGui.hpp
        layout.hpp
        font_louis_george.hpp
        CWidget.hpp
        CDeviceList.cpp
        CRSSIPlot.cpp
)


target_sources(${MAIN_PROJECT} PRIVATE ${GUI_SRCS})