SET(SECURITY_SRCS
        CWolfCrypt.cpp
        CRandom.cpp
        ecc_key.cpp
        CHash.hpp
        #CEccContext.cpp
        sha.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SECURITY_SRCS})
