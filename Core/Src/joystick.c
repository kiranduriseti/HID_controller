/*
 * joystick.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include <stdint.h>
#include "tim.h"
#include <stdio.h>
#include "adc.h"
#include "main_loop.h"
#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "buttons.h"
#include "joystick.h"

//lx : PA0
//ly : PA1
//rx : PA4
//ry : PA5

uint16_t joystick_adc[channels] = {0};
static int32_t joy_bias[channels] = {0};
int16_t lx, ly, rx, ry;

joystick_report report;

void joystick_calibrate(uint16_t samples)
{
    int64_t sum[channels] = {0};

    for (uint16_t i = 0; i < samples; i++) {
        for (uint8_t ch = 0; ch < channels; ch++) {
            sum[ch] += joystick_adc[ch];
        }
        HAL_Delay(2);
    }

    for (uint8_t ch = 0; ch < channels; ch++) {
        int32_t avg = sum[ch] / samples;
        joy_bias[ch] = avg - ADC_center;   // signed center error
    }
}

joystick_report get_report(void) {
	report.lx = lx;
	report.ly = ly;

	report.rx = rx;
	report.ry = ry;

	report.buttons = get_report_buttons();

	return report;
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
	//HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joystick_adc, channels);
	HAL_TIM_Base_Start(&htim3);
	joystick_calibrate(20);
}

//whenever circular buffer completes cycle
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	if (hadc->Instance == ADC1) {
		joystick_update();
	}
}

uint16_t joy_raw(uint8_t ch){
	if(ch >= channels) return 0;
	return joystick_adc[ch];
}

int16_t joy_signed(uint8_t ch) {
	if(ch >= channels) return 0;
	int32_t centered = (int32_t)joystick_adc[ch] - ADC_center;
	centered -= joy_bias[ch];
	return centered;
}


void joystick_print(void) {
	char msg[64];
	snprintf(msg, sizeof(msg), "lx: %d, ly: %d, rx: %d, ry: %d\n", lx, ly, rx, ry);
	printf(msg);
}

void joystick_update(void) {
	lx = deadzone_scale(joy_signed(0));
	ly = deadzone_scale(joy_signed(1));
	rx = deadzone_scale(joy_signed(2));
	ry = deadzone_scale(joy_signed(3));

//	lx = (joy_raw(0));
//	ly = (joy_raw(1));
//	rx = (joy_raw(2));
//	ry = (joy_raw(3));
}


