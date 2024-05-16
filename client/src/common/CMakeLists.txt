SET(COMMON_SRCS
        client_common.cpp
		client_common.hpp
        CStopWatch.hpp
        CThreadSafeHashMap.hpp
        CThreadSafeQueue.hpp
        CMutex.hpp
        CCoroutineManager.cpp
        CCoroutineManager.hpp
)


target_sources(${MAIN_PROJECT} PRIVATE ${COMMON_SRCS})