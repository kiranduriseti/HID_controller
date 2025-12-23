/*
 * main_loop.c
 *
 *  Created on: Dec 20, 2025
 *      Author: Kiran Duriseti
 */

#include "UART_print.h"
#include "usart.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"

#include "buttons.h"
#include "joystick.h"

int16_t lx, ly, rx, ry;

void joystick_loop(void) {
	lx = joy_signed(0);
	ly = joy_signed(1);
	rx = joy_signed(2);
	ry = joy_signed(3);

	if (fresh_data) {
		fresh_data = 0;
		if (fresh_data) {
		    fresh_data = 0;  // Reset the flag
		    printf("lx: %d, ly: %d, rx: %d, ry: %d\n", lx, ly, rx, ry);
		}

	}
}

void button_loop(void) {
	buttons_update();

	if (fresh_button) {
		read_button();
	}
}


void main_loop(void){

	UART_print("Hello World\r\n");
	UART_print("Testing Buffer\r\n");

	button_loop();
	joystick_loop();
}

