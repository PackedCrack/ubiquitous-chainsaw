#include "CGattService.hpp"


namespace application
{

namespace
{


[[Nodiscard]] ble_gatt_chr_def make_characteristic(const ble_uuid_t* uuid, ble_gatt_access_fn callbackFunc,
                                       ble_gatt_dsc_def* descriptors, uint8_t flags, uint16_t* handle) 
{
    return ble_gatt_chr_def {
        .uuid = uuid,
        .access_cb = callbackFunc,
        .descriptors = descriptors,
        .flags = flags,
        .val_handle = handle,
    };
}


[[Nodiscard]] ble_gatt_dsc_def make_descriptor(const ble_uuid_t* uuid, uint8_t attributeFlags, ble_gatt_access_fn callbackFunc) 
{
    return ble_gatt_dsc_def {
        .uuid = uuid,
        .att_flags = attributeFlags,
        .access_cb = callbackFunc,
    };
}

[[Nodiscard]] ble_gatt_svc_def make_service(uint8_t serviceType, const ble_uuid_t* uuid, ble_gatt_chr_def* characteristics) 
{
    return ble_gatt_svc_def {
        .type = serviceType,
        .uuid = uuid,
        .characteristics = characteristics,
    };
}


const ble_uuid128_t tmpServiceId =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);                    

uint8_t tmpCharacteristicValue;
uint16_t tmpCharacteristicValuehandle;
const ble_uuid128_t tmpCharacteristicId =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);


uint8_t tmpDescriptorValue;
const ble_uuid128_t tmpDescriptorId =
    BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);


int tmp_service_callback(uint16_t conn_handle, uint16_t attr_handle,
                            struct ble_gatt_access_ctxt *ctxt, void *arg)
{

    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        break;
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        break;
    case BLE_GATT_ACCESS_OP_READ_DSC:
        break;
    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        break;
    default:
        break;
    }
    assert(0);
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


struct std::array<ble_gatt_svc_def, 2> myService_TEMPLATE = 
{ 
    make_service(BLE_GATT_SVC_TYPE_PRIMARY, 
                    &tmpServiceId.u,
                    characteristics_TEMPLATE.data())
    , 0
}; 

} // namespace


CGattService::CGattService()
{

    ble_svc_gatt_init();

    int result = ble_gatts_count_cfg(myService_TEMPLATE.data());
    if (result != 0) {
        LOG_INFO("UNABLE TO COUNT GATT SVCS");
    }

    result = ble_gatts_add_svcs(myService_TEMPLATE.data());
    if (result != 0) {
        LOG_INFO("UNABLE TO ADD GATT SVCS");
    }

    /* Setting a value for the read-only descriptor */
    tmpDescriptorValue = 0x99; // why are we setting it here?
}

} // application namespace 