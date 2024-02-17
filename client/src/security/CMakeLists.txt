SET(SECURITY_SRCS
hash.cpp
CWolfCrypt.cpp)

target_sources(${MAIN_PROJECT} PRIVATE ${SECURITY_SRCS})