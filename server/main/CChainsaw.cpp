#include "CChainsaw.hpp"


namespace application
{ 

namespace 
{


constexpr std::string_view deviceName {"Chainsaw-server"};

uint8_t ble_generate_random_device_address() 
{
    int result;
    const int RND_ADDR = 1;
    const int PUB_ADDR = 0;
    const uint8_t INVALID_ADDRESS_TYPE = 255u;
    uint8_t addrType = INVALID_ADDRESS_TYPE;

    result = ble_hs_util_ensure_addr(RND_ADDR);
    assert(result == 0);
    result = ble_hs_id_infer_auto(PUB_ADDR, &addrType); // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != 0) 
        LOG_FATAL_FMT("No address was able to be inferred %d\n", result);
    
    if (addrType == INVALID_ADDRESS_TYPE)
        LOG_FATAL_FMT("Error address type not determined! %d\n", result);
    
    // print the address
    std::array<uint8_t, 6> bleDeviceAddr {};
    result = ble_hs_id_copy_addr(addrType, bleDeviceAddr.data(), NULL); 
    if (result != 0) 
        LOG_FATAL_FMT("Adress was unable to be assigned %d\n", result);

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", bleDeviceAddr[5],bleDeviceAddr[4],bleDeviceAddr[3],bleDeviceAddr[2],bleDeviceAddr[1],bleDeviceAddr[0]);

    return addrType;
}
} // namespace


void CChainsaw::start()
{
    // assert check so that sync is finished
    m_gapService.initilize(deviceName, ble_generate_random_device_address());
}

CChainsaw::CChainsaw()
    : m_gapService {}
    , m_gattService {}
{

    int result;
    result = ble_svc_gap_device_name_set(deviceName.data());
    assert(result == 0);

    //ble_svc_gap_init(); //register gap service to GATT server (service UUID 0x1800m, generic access, Device Name, Appearance READ ONlY for both
    //ble_svc_gatt_init(); // register GATT service to GATT server (service UUID 0x1801) // is this needed? yes, otherwise we cant add additional gatt services

}

} // namespace application