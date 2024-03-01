SET(COMMON_SRCS
        CStopWatch.hpp
        CThreadSafeHashMap.hpp
        CGenerator.hpp
)


target_sources(${MAIN_PROJECT} PRIVATE ${COMMON_SRCS})