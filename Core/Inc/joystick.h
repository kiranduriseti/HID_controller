/*
 * joystick.h
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#ifndef INC_JOYSTICK_H_
#define INC_JOYSTICK_H_

#include <stdint.h>
//extern volatile uint8_t fresh_data;

#define channels 4
#define ADC_max 4095
#define ADC_center (ADC_max/2 + 1)
#define deadzone (.05*ADC_max)
#define clamp 32767

typedef struct __attribute__((packed)){
	uint8_t buttons;

	int16_t lx;
	int16_t ly;

	int16_t rx;
	int16_t ry;
} joystick_report;

void joystick_start_scan(void);
void joystick_update(void);
void joystick_print(void);
int16_t joy_signed(uint8_t ch);
uint16_t joy_raw(uint8_t ch);
joystick_report get_report(void);
int16_t deadzone_scale(int32_t x);

#endif /* INC_JOYSTICK_H_ */
