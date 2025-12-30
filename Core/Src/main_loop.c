/*
 * main_loop.c
 *
 *  Created on: Dec 20, 2025
 *      Author: Kiran Duriseti
 */

#include "UART_print.h"
//#include "usart.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"

#include "buttons.h"
#include "joystick.h"
#include "usb_device.h"

#include "usbd_customhid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint32_t last_ms = 0;
uint8_t st;

void send_report(){
	buttons_update();
	//if (!(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)) return;

	uint32_t now = HAL_GetTick();
	if ((now - last_ms) < 5)
		return;
	last_ms = now;

	joystick_report report = get_report();
	st = USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&report, 9);
	//HAL_Delay(5);
}

void main_loop(void){
	send_report();
}

