/*
 * joystick.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include <stdint.h>
#include "tim.h"
#include "UART_print.h"
#include <stdio.h>
#include "adc.h"
#include "main_loop.h"
#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "buttons.h"

//lx : PA0
//ly : PA1
//rx : PA4
//ry : PA5

#define channels 4
#define ADC_max 4096
#define ADC_center (ADC_max/2)
#define deadzone (.015*ADC_max)
#define clamp 32767

extern USBD_HandleTypeDef hUsbDeviceFS;

//volatile uint8_t fresh_data;

uint16_t joystick_adc[channels] = {0};
int16_t lx, ly, rx, ry;

typedef struct {
	uint8_t buttons;

	int8_t lx;
	int8_t ly;

	int8_t rx;
	int8_t ry;
} joystickReport;

joystickReport report;

void send_report(void) {
	report.lx = lx;
	report.ly = ly;

	report.rx = rx;
	report.ry = ry;

	report.buttons = get_report_buttons();

	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&report, 5);
}

int16_t deadzone_scale(int32_t x){
	//ignore deadzone
	if(x < deadzone && x > -deadzone) return 0;

	//remove deadzone
	if (x > 0) x -= deadzone; else x += deadzone;

	int32_t denom = (ADC_center - 1 - deadzone);
	x =  (x * clamp)/denom;

	if (x > clamp) x = clamp; else if (x < -clamp) x = -clamp;

	return (int16_t)x;

}

void joystick_start_scan(void) {
	//fresh_data = 0;
	//HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joystick_adc, channels);
	HAL_TIM_Base_Start(&htim3);
}

//whenever circular buffer completes cycle
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	if (hadc->Instance == ADC1) {
//		fresh_data = 1;
		joystick_update();
		joystick_print();

		buttons_update();
		buttons_print();

		send_report();
	}
}

uint16_t joy_raw(uint8_t ch){
	if(ch >= channels) return 0;
	return joystick_adc[ch];
}

int16_t joy_signed(uint8_t ch) {
	if(ch >= channels) return 0;
	int32_t centered = (int32_t)joystick_adc[ch] - ADC_center;
	return centered;
}


void joystick_print(void) {
	char msg[64];
	snprintf(msg, sizeof(msg), "lx: %d, ly: %d, rx: %d, ry: %d\n", lx, ly, rx, ry);
	UART_print(msg);

}

void joystick_update(void) {
//	if (!fresh_data) return;
//	fresh_data = 0;
	lx = deadzone_scale(joy_signed(0));
	ly = deadzone_scale(joy_signed(1));
	rx = deadzone_scale(joy_signed(2));
	ry = deadzone_scale(joy_signed(3));
}


