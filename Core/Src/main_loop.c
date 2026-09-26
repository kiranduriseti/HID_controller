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
#include "controller_usb.h"
#include "stm32f4xx_hal.h"
#include <math.h>
#include "mpu.h"
#include "i2c.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static mode_gesture mode_hold;
uint8_t st;

int16_t gyro_x, gyro_y;
int16_t mpu_x, mpu_y, mpu_z;

HAL_StatusTypeDef debugger;
volatile uint32_t last_i2c_err = 0;
volatile HAL_StatusTypeDef r68 = HAL_OK;
volatile HAL_StatusTypeDef r69 = HAL_OK;
volatile uint8_t i2c_checked = 0;


#define deadzone_gyro 2.0
#define sens_gyro_x .85
#define sens_gyro_y .6
#define gyro_alpha .85
#define scaler 16.4f
#define low_mpu -200
#define high_mpu 200

int16_t bound (int x) {
	if (x > clamp) x = clamp; else if (x < -clamp) x = -clamp;
	return (int16_t)x;
}

joystick_report Jr_update(joystick_report report) {
	if (get_acc_state() == 1) return report;

	report.rx = bound(gyro_x + report.rx);
	report.ry = bound(gyro_y + report.ry);

	return report;
}

int clamp_mpu(int v){
  if (v < low_mpu) return low_mpu;
  if (v > high_mpu) return high_mpu;

  return v;
}

double deadzone_mpu(double v) {
  if (fabs(v) < deadzone_gyro) return 0.0;
  return v;
}

void mpu_joy(void){
    if (!mpu_gyro_ready() || mpu_get_status() != HAL_OK) {
        gyro_x = gyro_y = 0;
        return;
    }
	debugger = mpu_read_gyro(&mpu_x, &mpu_y);
    if (debugger != HAL_OK) {
        gyro_x = gyro_y = 0; // invalid sensor data must not keep steering the stick
        return;
    }

	float fx, fy;

	fx = (float)(mpu_x - gyro_bias_x)/scaler;
	fy = (float)(mpu_y - gyro_bias_y)/scaler;


	fx = deadzone_mpu(fx);
	fy = deadzone_mpu(fy);

	static float gx_f = 0.0f, gy_f = 0.0f;
	gx_f = gyro_alpha * gx_f + (1.0f - gyro_alpha) * fx;
	gy_f = gyro_alpha * gy_f + (1.0f - gyro_alpha) * fy;

	float gx_counts = (gx_f / (float)high_mpu) * (float)clamp;
	float gy_counts = (gy_f / (float)high_mpu) * (float)clamp;

	gyro_x = bound((int)(gy_counts * sens_gyro_x));
	gyro_y = bound((int)(gx_counts * sens_gyro_y));


}

void send_report(void){
    joystick_report report = get_report();
    report = Jr_update(report);
    if (mode_hold.latched || (report.buttons & MODE_BUTTONS) == MODE_BUTTONS)
        report.buttons &= (uint16_t)~MODE_BUTTONS;
    st = controller_usb_send(&report, HAL_GetTick());
}

void main_loop(void){

//	last_i2c_err = hi2c2.ErrorCode;
//	r68 = HAL_I2C_IsDeviceReady(&hi2c2, 0x68<<1, 3, 100);
//	r69 = HAL_I2C_IsDeviceReady(&hi2c2, 0x69<<1, 3, 100);
//	if (r68 == HAL_ERROR) {
//		HAL_I2C_DeInit(&hi2c2);
//		HAL_Delay(10);
//		HAL_I2C_Init(&hi2c2);
//		HAL_Delay(10);
//	}

	buttons_update();
    uint32_t now = HAL_GetTick();
    if (mode_gesture_update(&mode_hold, get_report_buttons(), now))
        controller_usb_toggle(now);
    controller_usb_poll(now);


	if (!get_acc_state()) mpu_joy();

	send_report();
}

