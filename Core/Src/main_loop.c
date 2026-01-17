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
#include "mpu.h"
#include "i2c.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

static uint32_t last_ms = 0;
uint8_t st;
double x, y, z;

int16_t gyro_x, gyro_y;
int16_t mpu_x, mpu_y, mpu_z;

HAL_StatusTypeDef debugger;
volatile uint32_t last_i2c_err = 0;
volatile HAL_StatusTypeDef r68 = HAL_OK;
volatile HAL_StatusTypeDef r69 = HAL_OK;
volatile uint8_t i2c_checked = 0;


int last_update = 0;
#define delta 5
int x_vel, y_vel;
int ax, ay;

#define acc_sens  800.0         // joystick counts per radian (tune)
#define alpha_lp  0.01        	// low-pass strength (0..1), higher = more responsive
#define dt_min_s  0.0005        // safety
#define dt_max_s  0.050         // safety

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

	//ax = deadzone_scale(x_vel*(ADC_max));
	//ay = deadzone_scale(y_vel*(ADC_max));
//	ax = deadzone_scale(x_vel);
//	ay = deadzone_scale(y_vel);

//	report.rx = bound(ax + report.rx);
//	report.ry = bound(ay + report.ry);
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
//	int now = HAL_GetTick();
//	if (now - last_update < delta) return;

	debugger = mpu_read_gyro(&mpu_x, &mpu_y);
	//last_update = now;

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

void ACC_joy(void) {
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

void send_report(void){
	//if (!(hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)) return;
	uint32_t now = HAL_GetTick();
	if ((now - last_ms) < 5)
		return;
	last_ms = now;

	joystick_report report = get_report();
	report = Jr_update(report);
	st = USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&report, sizeof(report));
	//HAL_Delay(5);
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

	//if (!get_acc_state()) ACC_joy();

	if (!get_acc_state()) mpu_joy();

	send_report();
}

