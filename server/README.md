
# Ubuquitous-Chainsaw Server Configuration

## Prerequisites
- Espressif installed on your system, preferably the ESP-IDF extension for Visual Studio Code

## Setup project
### Using Visual Studio Code (VSC)
- Open server directory in/using VSC
- Open ESP-IDF Terminal (located at the bottom toolbar)
- Run the command: python ConfigureServer.py



#################
UNUSED CLASS
-------------
Checking C:\skolarbete\thesis\ubiquitous-chainsaw\shared\security\CEccContext.hpp ...
C:\skolarbete\thesis\ubiquitous-chainsaw\shared\security\CEccContext.hpp:98:14: style: class member 'CEccContext::m_EncAlg' is never used. [unusedStructMember]
    EcEncAlg m_EncAlg;
             ^
C:\skolarbete\thesis\ubiquitous-chainsaw\shared\security\CEccContext.hpp:99:14: style: class member 'CEccContext::m_KdfAlg' is never used. [unusedStructMember]
    EcKdfAlg m_KdfAlg;
             ^
C:\skolarbete\thesis\ubiquitous-chainsaw\shared\security\CEccContext.hpp:100:14: style: class member 'CEccContext::m_MacAlg' is never used. [unusedStructMember]
    EcMacAlg m_MacAlg;



THIS IS USED
    C:\skolarbete\thesis\ubiquitous-chainsaw\client\src\gui\CDeviceList.hpp:37:34: style: class member 'CDeviceList::m_Devices' is never used. [unusedStructMember]
    std::vector<ble::DeviceInfo> m_Devices;
                                 ^

C:\skolarbete\thesis\ubiquitous-chainsaw\client\src\bluetoothLE\windows\CService.hpp:34:71: style: class member 'CService::m_Characteristics' is never used. [unusedStructMember]
    std::unordered_map<ble::UUID, CCharacteristic, ble::UUID::Hasher> m_Characteristics;


C:\skolarbete\thesis\ubiquitous-chainsaw\client\src\bluetoothLE\windows\CDevice.hpp:42:25: style: class member 'CDevice::m_Services' is never used. [unusedStructMember]
    service_container_t m_Services;

    