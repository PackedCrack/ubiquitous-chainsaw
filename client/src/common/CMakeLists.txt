SET(COMMON_SRCS
        CStopWatch.hpp
        CThreadSafeHashMap.hpp
        CMutex.hpp
)


target_sources(${MAIN_PROJECT} PRIVATE ${COMMON_SRCS})