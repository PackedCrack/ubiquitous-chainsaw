SET(SECURITY_SRCS
        CWolfCrypt.cpp
        CRandom.cpp
        CEccKey.cpp
        CHash.hpp
        sha.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SECURITY_SRCS})