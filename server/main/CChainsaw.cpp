#include "CChainsaw.hpp"


namespace application
{ 
static constexpr uint8_t ADDRESS_TYPE_ERROR = 255; // moev this to a seperate ChainsawDefines along with related headers???

namespace 
{

constexpr std::string_view deviceName {"Chainsaw-server"};

uint8_t ble_generate_random_device_address() 
{
    int result;
    uint8_t addrType = ADDRESS_TYPE_ERROR;
    const int RND_ADDR = 1;
    const int PUB_ADDR = 0;

    result = ble_hs_util_ensure_addr(RND_ADDR);
    assert(result == 0);
    result = ble_hs_id_infer_auto(PUB_ADDR, &addrType); // 1/private do not work here, type will depend ble_hs_util_ensure_addr()
    if (result != 0) 
        LOG_FATAL_FMT("No address was able to be inferred %d\n", result);
    
    if (addrType == ADDRESS_TYPE_ERROR)
        LOG_FATAL_FMT("Error address type not determined! %d\n", result);
    
    // print the address
    std::array<uint8_t, 6> bleDeviceAddr {}; // what do you mean incomplete type?
    result = ble_hs_id_copy_addr(addrType, bleDeviceAddr.data(), NULL); 
    if (result != 0) 
        LOG_FATAL_FMT("Adress was unable to be assigned %d\n", result);

    std::printf("BLE Device Address: %02x:%02x:%02x:%02x:%02x:%02x \n", bleDeviceAddr[5],bleDeviceAddr[4],bleDeviceAddr[3],bleDeviceAddr[2],bleDeviceAddr[1],bleDeviceAddr[0]);

    return addrType;
}

} // namespace

void CChainsaw::gap_start_advertise()
{
    m_gapService.start_advertise();
}

void CChainsaw::gap_stop_advertise()
{
    m_gapService.stop_advertise();
}


void CChainsaw::start()
{
    // assert check so that sync is finished

    m_bleAddressType = ble_generate_random_device_address();
    assert(m_bleAddressType != ADDRESS_TYPE_ERROR);
    m_gapService.initilize(deviceName, m_bleAddressType); // todo add functor for callback

    gap_start_advertise();

}

CChainsaw::CChainsaw()
    : m_bleAddressType {255} // This value needs to be invalid to start with for error checking
    , m_gapService {}
{

    int result;
    result = ble_svc_gap_device_name_set(deviceName.data());
    assert(result == 0);

    ble_svc_gap_init(); //register gap service to GATT server (service UUID 0x1800m, generic access, Device Name, Appearance READ ONlY for both
    //ble_svc_gatt_init(); // register GATT service to GATT server (service UUID 0x1801) // is this needed? yes, otherwise we cant add additional gatt services

    // init custom gatt services here

}

} // namespace application