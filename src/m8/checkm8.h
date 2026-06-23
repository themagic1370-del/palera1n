#ifndef CHEKMC8_H
#define CHEKMC8_H

#include "payload.h"

#ifndef __APPLE__
# include "usb-libusb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

bool checkm8_reset(usb_handle_t *handle);
bool checkm8_stall(usb_handle_t *handle);
bool checkm8_usb_request_stall(usb_handle_t *handle);
bool checkm8_send_leaking_zlp(usb_handle_t *handle);
bool checkm8_send_normal_zlp(usb_handle_t *handle);
bool checkm8_heap_feng_shui(usb_handle_t *handle, struct DeviceConfiguration *config);
bool checkm8_trigger_UaF(usb_handle_t *handle, struct DeviceConfiguration *config);

bool checkm8_send_overwrite_and_payload(usb_handle_t *handle, struct DeviceConfiguration *deviceConfig, struct PayloadConfiguration *payloadConfig);

#ifdef __cplusplus
}
#endif

#endif // CHEKMC8_H

