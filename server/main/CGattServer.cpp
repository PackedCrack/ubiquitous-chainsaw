#include "CGattServer.hpp"




namespace nimble
{


CGattServer::CGattServer(const std::string_view deviceName, const uint8_t addrType) 
    :m_gap{deviceName, addrType} 
{
}

[[NoDiscard]] uint8_t CGattServer::gap_param_is_alive()
{
    return m_gap.gap_param_is_alive();
    
}


} // namesapce nimble