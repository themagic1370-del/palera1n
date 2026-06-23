#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef __APPLE__
# include "usb-libusb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct DeviceConfiguration {
    uint64_t cpid, largeLeak, overwritePadding, hole;
};

struct PayloadConfiguration {
    uint64_t tlbi, nopGadget, retGadget, patchAddress, ttbr0Address, functionGadget, ttbr0Write,
    memcpyAddress, aesCryptoCommand, bootTrampolineEnd, ttbr0VROMOffset, ttbr0SRAMOffset,
    gUSBSerialNumber, dfu_handle_request_address, usb_core_do_transfer, dfu_handle_bus_reset_address,
    insecureMemoryBase, handle_interface_request, usb_create_string_descriptor, usb_serial_number_string_descriptor;
};

bool checkm8_find_device_configuration_for_cpid(int cpid, struct DeviceConfiguration *config);
bool checkm8_find_payload_configuration_for_cpid(int cpid, struct PayloadConfiguration *config);
uint8_t *create_pongo_overwrite_for_device(struct PayloadConfiguration *payloadConfig, size_t *overwriteSize);
uint8_t *create_pongo_payload_for_device(struct DeviceConfiguration *deviceConfig, size_t *payloadSize);

// TODO? move
bool wait_for_device_to_enter_yolo_dfu(usb_handle_t *handle);
bool send_pongo_to_yolo_dfu(usb_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif // PAYLOAD_H
