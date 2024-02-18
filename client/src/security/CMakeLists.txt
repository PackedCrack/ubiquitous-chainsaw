SET(SECURITY_SRCS
hash.cpp
CWolfCrypt.cpp
CRandom.cpp
CEccKey.cpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SECURITY_SRCS})