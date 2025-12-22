/*
 * joystick.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include <stdint.h>
#include "UART_print.h"
#include <stdio.h>
#include "adc.h"
#include "stm32f4xx_hal.h"


//lx : PA0
//ly : PA1
//rx : PA4
//ry : PA5

#define channels 4
#define ADC_max 4096
#define ADC_center (ADC_max/2)
#define deadzone (.015*ADC_max)
#define clamp 32767

volatile uint8_t fresh_data;

uint16_t joystick_adc[channels] = {0};

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
	fresh_data = 0;
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)joystick_adc, channels);
}

//whenever circular buffer completes cycle
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
	if (hadc->Instance == ADC1) {
		fresh_data = 1;
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



