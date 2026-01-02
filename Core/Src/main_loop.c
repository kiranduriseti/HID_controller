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
#include "accelerometer.h"
#include "stm32f4xx_hal.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint32_t last_ms = 0;
uint8_t st;
double x, y, z;

int last_update = 0;
#define delta 5
#define acc_sens 10
int x_vel, y_vel;
int ax, ay;

int16_t bound (int x) {
	if (x > clamp) x = clamp; else if (x < -clamp) x = -clamp;
	return x;
}

joystick_report Jr_update(joystick_report report) {
	if (get_acc_state() == 0) return report;

	ax = deadzone_scale(x_vel*(4095));
	ay = deadzone_scale(y_vel*(4095));

	report.lx = bound(ax + report.lx);
	report.ly = bound(ay + report.ly);

	return report;
}

void ACC_joy() {
	int now = HAL_GetTick();
	if (now - last_update > delta) {
		double dt = (now - last_update)/1000.0;

		last_update = now;

		x_vel += x * dt;
		y_vel += y * dt;
	}
}

void send_report(){
	//if (!(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)) return;
	uint32_t now = HAL_GetTick();
	if ((now - last_ms) < 5)
		return;
	last_ms = now;

	joystick_report report = get_report();
	report = Jr_update(report);
	st = USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&report, 9);
	//HAL_Delay(5);
}

void main_loop(void){
	readValues(&x, &y, &z);
	ACC_joy();
	buttons_update();

	send_report();
}

