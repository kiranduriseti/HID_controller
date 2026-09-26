#ifndef CONTROLLER_USB_H
#define CONTROLLER_USB_H
#include "controller_logic.h"
controller_mode controller_usb_mode(void);
void controller_usb_toggle(uint32_t now);
void controller_usb_poll(uint32_t now);
uint8_t controller_usb_send(const joystick_report *state, uint32_t now);
void controller_usb_init(void);
uint16_t controller_usb_report_descriptor_size(void);
uint8_t *controller_usb_configuration(uint16_t *length);
uint8_t *controller_usb_hid_descriptor(void);
#endif
