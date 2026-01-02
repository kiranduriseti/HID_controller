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
#include <math.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint32_t last_ms = 0;
uint8_t st;
double x, y, z;

int last_update = 0;
#define delta 5
int x_vel, y_vel;
int ax, ay;

#define acc_sens  800.0         // joystick counts per radian (tune)
#define alpha_lp  0.15          // low-pass strength (0..1), higher = more responsive
#define dt_min_s  0.0005        // safety
#define dt_max_s  0.050         // safety

int16_t bound (int x) {
	if (x > clamp) x = clamp; else if (x < -clamp) x = -clamp;
	return (int16_t)x;
}

joystick_report Jr_update(joystick_report report) {
	if (get_acc_state() == 1) return report;

	//ax = deadzone_scale(x_vel*(ADC_max));
	//ay = deadzone_scale(y_vel*(ADC_max));
	ax = deadzone_scale(x_vel);
	ay = deadzone_scale(y_vel);

	report.rx = bound(ax + report.rx);
	report.ry = bound(ay + report.ry);

	return report;
}

void ACC_joy() {
	int now = HAL_GetTick();
	if (now - last_update < delta) return;

	double dt = (now - last_update)/1000.0;

	readValues(&x, &y, &z);
	last_update = now;

	if (dt < dt_min_s) dt = dt_min_s;
	if (dt > dt_max_s) dt = dt_max_s;

	 /* --- tilt estimate from gravity vector --- */
	double roll  = atan2(y, z);
	double pitch = atan2(-x, sqrt(y*y + z*z));

	/* convert radians -> joystick counts */
	int raw_ax = (int)(acc_sens * roll);
	int raw_ay = (int)(acc_sens * pitch);

	x_vel += x * dt;
	y_vel += y * dt;

	/*
	 * Low-pass filter (EMA):
	 * filtered = filtered + alpha*(raw - filtered)
	 * Using your x_vel/y_vel variables as the filtered offsets.
	 */
	x_vel = (int)((double)x_vel + alpha_lp * ((double)raw_ax - (double)x_vel));
	y_vel = (int)((double)y_vel + alpha_lp * ((double)raw_ay - (double)y_vel));

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
	buttons_update();

	ACC_joy();

	send_report();
}

