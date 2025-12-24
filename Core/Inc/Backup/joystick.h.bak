/*
 * joystick.h
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#ifndef INC_JOYSTICK_H_
#define INC_JOYSTICK_H_

#include <stdint.h>
extern volatile uint8_t fresh_data;

void joystick_start_scan(void);
void joystick_update(void);
void joystick_print(void);
int16_t joy_signed(uint8_t ch);
uint16_t joy_raw(uint8_t ch);

#endif /* INC_JOYSTICK_H_ */
