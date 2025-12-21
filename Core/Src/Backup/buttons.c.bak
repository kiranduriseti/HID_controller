/*
 * buttons.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include "UART_print.h"
#include <string.h>
//ABXY+-
int buttons [] = {0, 0, 0, 0, 0, 0};
int debounce_time = {0, 0, 0, 0, 0, 0};

#define debounce 10

#define A GPIO_PIN_3
#define B GPIO_PIN_4
#define X GPIO_PIN_5
#define Y GPIO_PIN_6
#define plus GPIO_PIN_8
#define minus GPIO_PIN_9

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	uint32_t now = HAL_GetTick();

	if(GPIO_Pin == A) {
		if (now - debounce_time[0] > debounce){
			debounce_time[0] = now;
			buttons[0] ^= 1;
		}
	}
	if(GPIO_Pin == B) {
		if (now - debounce_time[1] > debounce){
			debounce_time[1] = now;
			buttons[1] ^= 1;
		}
	}
	if(GPIO_Pin == X) {
		if (now - debounce_time[2] > debounce){
			debounce_time[2] = now;
			buttons[2] ^= 1;
		}
	}
	if(GPIO_Pin == Y) {
		if (now - debounce_time[3] > debounce){
			debounce_time[3] = now;
			buttons[3] ^= 1;
		}
	}
	if(GPIO_Pin == plus) {
		if (now - debounce_time[3] > debounce){
			debounce_time[3] = now;
			buttons[3] ^= 1;
		}
	}
	if(GPIO_Pin == minus) {
		if (now - debounce_time[3] > debounce){
			debounce_time[3] = now;
			buttons[3] ^= 1;
		}
	}
}

void read_button(void){
	char msg [] = buttons;
	UART_print(msg);
}
