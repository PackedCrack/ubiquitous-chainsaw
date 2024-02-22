#include "CGatt.hpp"

#define BLE_DYNAMIC_SERVICE_DELETE 0


namespace ble
{

namespace
{


[[Nodiscard]] const ble_gatt_dsc_def make_descriptor(const ble_uuid_t* pUuid, uint8_t attributeFlags, ble_gatt_access_fn callbackFunc) 
{
    return ble_gatt_dsc_def {
        .uuid = pUuid,
        .att_flags = attributeFlags,
        .access_cb = callbackFunc,
    };
}


[[Nodiscard]] const ble_gatt_chr_def make_characteristic(const ble_uuid_t* pUuid, ble_gatt_access_fn callbackFunc,
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


[[Nodiscard]] const ble_gatt_svc_def make_service(uint8_t serviceType, const ble_uuid_t* pUuid, ble_gatt_chr_def* pCharacteristics) 
{
    return ble_gatt_svc_def {
        .type = serviceType,
        .uuid = pUuid,
        .characteristics = pCharacteristics,
    };
}


uint8_t tmpDescriptorValue; 
const ble_uuid128_t tmpDescriptorId = BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                                                       0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);                 

uint16_t tmpCharacteristicValue;
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


struct std::array<ble_gatt_dsc_def, 2> descriptors_TEMPLATE = 
{ 
    make_descriptor( &tmpDescriptorId.u,
                        BLE_ATT_F_READ,
                        tmp_service_callback)
    , 0
};


struct std::array<ble_gatt_chr_def, 2> characteristics_TEMPLATE = 
{ 
    make_characteristic(&tmpCharacteristicId.u,
                           tmp_service_callback,
                           descriptors_TEMPLATE.data(),
                           BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY | BLE_GATT_CHR_F_INDICATE,
                           &tmpCharacteristicValuehandle)
    , 0
};


const struct std::array<ble_gatt_svc_def, 2> myService_TEMPLATE = 
{ 
    make_service(BLE_GATT_SVC_TYPE_PRIMARY, 
                    &tmpServiceId.u,
                    characteristics_TEMPLATE.data())
    , 0
}; 

int dynamic_service(const uint8_t operation, const struct ble_gatt_svc_def *svcs, const ble_uuid_t *uuid) {
    int rc = 0;
    int i = 0;
    switch(operation) {
     
        case 0:
        int rc;
        rc = ble_gatts_delete_svc(uuid);
        if(rc != 0) {
            /* not able to delete service return immidietely */
            return rc;
        }
        i++;
        return rc;
            break;
    }
    return rc;
}

} // namespace



CGatt::CGatt()
{
    //throw std::runtime_error("An error occured during Construction of CGatt!");

}

void CGatt::register_services()
{

    
    //ble_svc_gatt_init();

    int result = ble_gatts_count_cfg(myService_TEMPLATE.data());
    if (result != 0) {
        LOG_INFO("UNABLE TO COUNT GATT SVCS");
    }

    result = ble_gatts_add_svcs(myService_TEMPLATE.data());
    if (result != 0) {
        LOG_INFO("UNABLE TO ADD GATT SVCS");
    }
}


} // application namespace 
