SET(SECURITY_SRCS
        wc_defines.cpp
        wc_defines.hpp
        CWolfCrypt.cpp
        CWolfCrypt.hpp
        CRandom.cpp
        CRandom.hpp
        ecc_key.cpp
        ecc_key.hpp
        CHash.hpp
        sha.cpp
        sha.hpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SECURITY_SRCS})