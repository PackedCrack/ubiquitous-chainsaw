#include "CGatt.hpp"

namespace ble
{

namespace
{



const ble_gatt_dsc_def make_descriptor(const ble_uuid_t* pUuid, uint8_t attributeFlags, ble_gatt_access_fn callbackFunc) 
{
    return ble_gatt_dsc_def {
        .uuid = pUuid,
        .att_flags = attributeFlags,
        .access_cb = callbackFunc,
    };
}


const ble_gatt_chr_def make_characteristic(const ble_uuid_t* pUuid, ble_gatt_access_fn callbackFunc,
                                       ble_gatt_dsc_def* pDescriptors, uint8_t flags, uint16_t* pHandle) 
{
    return ble_gatt_chr_def {
        .uuid = pUuid,
        .access_cb = callbackFunc,
        .descriptors = pDescriptors,
        .flags = flags,
        .val_handle = pHandle,
    };
}


const ble_gatt_svc_def make_service(uint8_t serviceType, const ble_uuid_t* pUuid, ble_gatt_chr_def* pCharacteristics) 
{
    return ble_gatt_svc_def {
        .type = serviceType,
        .uuid = pUuid,
        .characteristics = pCharacteristics,
    };
}


//uint8_t tmpDescriptorValue; 
const ble_uuid128_t tmpDescriptorId = BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                                                       0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);                 

//uint16_t tmpCharacteristicValue;
uint16_t tmpCharacteristicValuehandle;
const ble_uuid128_t tmpCharacteristicId = BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                                                           0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

const ble_uuid128_t tmpServiceId = BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                                                    0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);  




std::vector<uint8_t> writeOperationData {};

int tmp_service_callback(uint16_t connectionHandle, uint16_t attributeHandle, // attributeHandle means both characterisitics and descriptor, depending on who triggered the callback
                            struct ble_gatt_access_ctxt* pContext, void* pArg)
{
    switch (pContext->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR: 
    {
        LOG_INFO("BLE_GATT_ACCESS_OP_READ_CHR");



        if (connectionHandle == BLE_HS_CONN_HANDLE_NONE)
         {
            LOG_ERROR("NO CONNECTION EXISTS FOR READ CHARACTERISTIC!");
            return BLE_ATT_ERR_INSUFFICIENT_RES;
         }

        LOG_INFO_FMT("Characteristic read. conn_handle={} attr_handle={}\n", connectionHandle, attributeHandle);

        if (writeOperationData.empty())
        {
            writeOperationData.push_back(69);
        }

        
        if (attributeHandle == tmpCharacteristicValuehandle) 
        {
            std::printf("length of data: %d", sizeof(writeOperationData));
            int result = os_mbuf_append(pContext->om,
                                writeOperationData.data(),
                                writeOperationData.size());

            return result == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        else 
        {
            return BLE_ATT_ERR_INVALID_HANDLE;
        }
    }
    break;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
    {
        /*
            The server determines if the characteristic value is updated or not
            meaning, no external party can modify the servers internal state
            (the value doesnt have to be updated btw)
        */

       /*
       Todo handle errors
       */

        LOG_INFO("BLE_GATT_ACCESS_OP_WRITE_CHR");
        const int MAX_UUID_LEN = 128;
        char uuidBuf [MAX_UUID_LEN];
        std::string_view charUuid = ble_uuid_to_str((const ble_uuid_t* )&pContext->chr->uuid, uuidBuf);


        if (pContext->om == nullptr || pContext->om->om_len < 1)
        {
            LOG_INFO("NO DATA WAS WRITTEN!");
            break;
        }
        
        // Read the data from the buffer, it expects BYTE ARRAY or BYTE, otherwise extra care have to be made if its uint or int or UTF-8 (text)
        /*
        writing "hej"
        Data read[19]: 68 -> 'h'
        Data read[20]: 65 -> 'e'
        Data read[21]: 6a -> 'j'

        Writing 999999 UINT 32 (Little Endian)
        Data read[19]: 00
        Data read[20]: 0f
        Data read[21]: 42
        Data read[22]: 3f

        Writing 999999 UINT 32 (Big Endian)   <---  WE EXPECT Big Endian format
        Data read[19]: 3f
        Data read[20]: 42
        Data read[21]: 0f
        Data read[22]: 00

        */
        writeOperationData.clear();
        uint8_t* pDataBuffer = pContext->om->om_databuf;

        // TODO check if this is always the case! 
        const uint16_t DATA_DELIMITER = 19;
        const uint8_t NUM_DATA = *pDataBuffer;
        const uint8_t DATA_END = DATA_DELIMITER + NUM_DATA;

        LOG_INFO_FMT("{} bytes was written to characteristic={}", NUM_DATA, charUuid);
        for (int i = DATA_DELIMITER; i < DATA_END; ++i)
        {
            std::printf("Data read[%d]: %02x\n", i, pDataBuffer[i]);
        }

        // save the data in a vector
        for (int i = DATA_DELIMITER; i < DATA_END; ++i) 
        {
            writeOperationData.push_back(pDataBuffer[i]);
        }
        break;
    }
    case BLE_GATT_ACCESS_OP_READ_DSC:
    LOG_INFO("BLE_GATT_ACCESS_OP_READ_DSC");
        break;
    case BLE_GATT_ACCESS_OP_WRITE_DSC:
    LOG_INFO("BLE_GATT_ACCESS_OP_WRITE_DSC");
        break;
    default:
        assert(0); // restarts the device
        break;
    }

    // should never trigger
    return BLE_ATT_ERR_UNLIKELY;
}

BleService tmp = {
 .desc = {
     make_descriptor(&tmpDescriptorId.u, BLE_ATT_F_READ, tmp_service_callback)
     , 0
 },
 .chr = {
     make_characteristic(&tmpCharacteristicId.u, tmp_service_callback, tmp.desc.data(),
                        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE, &tmpCharacteristicValuehandle)
     , 0
 },
 .svc = {
     make_service(BLE_GATT_SVC_TYPE_PRIMARY, &tmpServiceId.u, tmp.chr.data())
    , 0
 }
};


} // namespace


CGatt::CGatt()
: m_services {std::move(tmp)} // How to solve this in a better way?
{
    //throw std::runtime_error("An error occured during Construction of CGatt!");
}


CGatt::~CGatt()
{
    std::printf("CGatt destructor\n");
    //int result = ble_gatts_reset(); // BLE_HS_EBUSY 
    //ASSERT(result == 0, "Unable to deconstruct CGatt due to existing connections or active GAP procedures.");
}

//CGatt::CGatt(CGatt&& other) noexcept 
//    : m_services { std::move(other.m_services) }
//{
//   
//    const int NUM_DESC = other.m_services.desc.size();
//    for (int i = 0; i < NUM_DESC; ++i)
//    {
//        other.m_services.desc[i].uuid = nullptr;
//        other.m_services.desc[i].access_cb = nullptr;
//        other.m_services.desc[i].arg = nullptr;
//    }
//
//    const int NUM_CHR = other.m_services.chr.size();
//    for (int i = 0; i < NUM_CHR; ++i)
//    {
//        other.m_services.chr[i].uuid = nullptr;
//        other.m_services.chr[i].access_cb = nullptr;
//        other.m_services.chr[i].arg = nullptr;
//        other.m_services.chr[i].descriptors = nullptr;
//        other.m_services.chr[i].val_handle = nullptr;
//    }
//
//    const int NUM_SVC = other.m_services.svc.size();
//    for (int i = 0; i < NUM_SVC; ++i)
//    {
//        other.m_services.svc[i].uuid = nullptr;
//        other.m_services.svc[i].includes = nullptr;
//        other.m_services.svc[i].characteristics = nullptr;
//    }
//};
//
//
//CGatt& CGatt::operator=(CGatt&& other)
//{
//
//    // clean up visible resources
//    const int NUM_DESC = m_services.desc.size();
//    for (int i = 0; i < NUM_DESC; ++i)
//    {
//        delete m_services.desc[i].uuid;
//    }
//
//    const int NUM_CHR = m_services.chr.size();
//    for (int i = 0; i < NUM_CHR; ++i)
//    {
//        delete m_services.chr[i].uuid;
//        delete m_services.chr[i].descriptors;
//        delete m_services.chr[i].val_handle;
//    }
//
//    const int NUM_SVC = m_services.svc.size();
//    for (int i = 0; i < NUM_SVC; ++i)
//    {
//        delete m_services.svc[i].uuid;
//        delete m_services.svc[i].includes;
//        delete m_services.svc[i].characteristics;
//    }
//    
//    // transfer contents
//    m_services = std::move(other.m_services);
//
//    // nullify ptr's in other
//    for (int i = 0; i < NUM_DESC; ++i)
//    {
//        other.m_services.desc[i].uuid = nullptr;
//        other.m_services.desc[i].access_cb = nullptr;
//        other.m_services.desc[i].arg = nullptr;
//    }
//
//    for (int i = 0; i < NUM_CHR; ++i)
//    {
//        other.m_services.chr[i].uuid = nullptr;
//        other.m_services.chr[i].access_cb = nullptr;
//        other.m_services.chr[i].arg = nullptr;
//        other.m_services.chr[i].descriptors = nullptr;
//        other.m_services.chr[i].val_handle = nullptr;
//    }
//
//    for (int i = 0; i < NUM_SVC; ++i)
//    {
//        other.m_services.svc[i].uuid = nullptr;
//        other.m_services.svc[i].includes = nullptr;
//        other.m_services.svc[i].characteristics = nullptr;
//    }
//
//    return *this;
//}


// This does not work. Soft reset is triggered
//CGatt::CGatt()
//{
//    //throw std::runtime_error("An error occured during Construction of CGatt!");
//
//   BleService tmp = {
//    .desc = {
//        make_descriptor(&tmpDescriptorId.u, BLE_ATT_F_READ, tmp_service_callback)
//        , 0
//    },
//    .chr = {
//        make_characteristic(&tmpCharacteristicId.u, tmp_service_callback, tmp.desc.data(),
//                           BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE, &tmpCharacteristicValuehandle)
//        , 0
//    },
//    .svc = {
//        make_service(BLE_GATT_SVC_TYPE_PRIMARY, &tmpServiceId.u, tmp.chr.data())
//    , 0
//    }
//};
//
//m_services[0] = std::move(tmp);
//}


int CGatt::register_services()
{

    //ble_svc_gatt_init();

    int result = ble_gatts_reset(); // BLE_HS_EBUSY 
    if (result != SUCCESS)
        return result;
    
    result = ble_gatts_count_cfg(m_services.svc.data()); // BLE_HS_EINVAL 
    if (result != SUCCESS) 
        return result;

    return ble_gatts_add_svcs(m_services.svc.data()); // BLE_HS_ENOMEM 
}


} // application namespace 
