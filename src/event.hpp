#ifdef WITH_GUI

#ifndef EVENT_H
#define EVENT_H

#include "state.hpp"

#include <wx/wx.h>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

class MainFrame;

void normal_device_event_cb(const idevice_event_t* event, void* user_data);
void recovery_device_event_cb(const irecv_device_event_t* event, void* user_data);

#endif // EVENT_H

#endif // WITH_GUI
