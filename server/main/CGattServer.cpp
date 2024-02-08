#include "CGattServer.hpp"




namespace nimble
{


CGattServer::CGattServer(const std::string_view deviceName, const uint8_t addrType) 
    :m_gap{deviceName, addrType} 
{
}


} // namesapce nimble