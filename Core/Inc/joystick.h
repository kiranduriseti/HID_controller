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

#endif /* INC_JOYSTICK_H_ */
