#pragma once
// third-part
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "common/serial_communication.hpp"
//
//
//
//
namespace usb
{
struct DeviceDescriptorSettings
{
    uint16_t vendorID;
    uint16_t productID;
    uint16_t deviceVersion;       // as binary coded decimal
    uint8_t manufactoryIndex;     // Index for the string containing the manufacturer name
    uint8_t productIndex;         // Index for the string containing the product name
    uint8_t serialNumberIndex;    // Index for the string containing the serial number
};
[[nodiscard]] consteval tinyusb_config_cdcacm_t make_cdc_config()
{
    return tinyusb_config_cdcacm_t{ .usb_dev = TINYUSB_USBDEV_0,
                                    .cdc_port = TINYUSB_CDC_ACM_0,
                                    .rx_unread_buf_sz = 64,
                                    .callback_rx = nullptr,
                                    .callback_rx_wanted_char = nullptr,
                                    .callback_line_state_changed = nullptr,
                                    .callback_line_coding_changed = nullptr };
}
[[nodiscard]] consteval tusb_desc_device_t make_device_descriptor(const DeviceDescriptorSettings& settings)
{
    return tusb_desc_device_t{
        .bLength = sizeof(tusb_desc_device_t),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x02'00,    // USB specification version - 2.0
        .bDeviceClass = TUSB_CLASS_CDC,
        .bDeviceSubClass = MISC_SUBCLASS_COMMON,
        .bDeviceProtocol = MISC_PROTOCOL_IAD,
        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor = settings.vendorID,
        .idProduct = settings.productID,
        .bcdDevice = settings.deviceVersion,    // Device's firmware version - 1.0
        // Indexing string descriptors start at 1. Index 0 is reserved for indicating that there are no string descriptors - I think
        .iManufacturer = settings.manufactoryIndex,    // Index for the string containing the manufacturer name
        .iProduct = settings.productIndex,
        .iSerialNumber = settings.serialNumberIndex,
        .bNumConfigurations = 1
    };
}
[[nodiscard]] consteval tusb_desc_device_t make_device_descriptor()
{
    constexpr usb::DeviceDescriptorSettings settings{ .vendorID = 0xDE'AD,
                                                      .productID = 0x13'37,
                                                      .deviceVersion = 0x01'00,
                                                      .manufactoryIndex = 0x00,
                                                      .productIndex = 0x01,
                                                      .serialNumberIndex = 0x02 };
    constexpr tusb_desc_device_t deviceDescriptor = usb::make_device_descriptor(settings);

    return deviceDescriptor;
}
[[nodiscard]] tinyusb_config_t make_config(const tusb_desc_device_t& deviceDescriptor, std::array<const char*, 3>& stringDescriptors);
void init_usb(const tinyusb_config_t& config, const tinyusb_config_cdcacm_t& cdcConfig);
}    // namespace usb
