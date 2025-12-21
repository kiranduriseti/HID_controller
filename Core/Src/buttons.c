/*
 * buttons.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include "UART_print.h"
#include <string.h>
//ABXY+-
uint32_t buttons [] = {0, 0, 0, 0, 0, 0};
uint32_t debounce_time [] = {0, 0, 0, 0, 0, 0};

#define debounce 10

#define A GPIO_PIN_3
#define B GPIO_PIN_4
#define X GPIO_PIN_5
#define Y GPIO_PIN_6
#define plus GPIO_PIN_8
#define minus GPIO_PIN_9

int state = 0;
void InterruptHelper(uint16_t GPIO_PIN, uint8_t index){
	uint32_t now = HAL_GetTick();

	if (now - debounce_time[index] > debounce){
		uint8_t level = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN);
		state = (level == 0) ? 1 : 0;

		if (state) {
			buttons[index] = 1;
		}
		else {
			buttons[index] = 0;
		}

		debounce_time[index] = now;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == A) {
		InterruptHelper(A, 0);
	}

	if(GPIO_Pin == B) {
		InterruptHelper(B, 1);
	}
	if(GPIO_Pin == X) {
		InterruptHelper(X, 2);
	}
	if(GPIO_Pin == Y) {
		InterruptHelper(Y, 3);
	}
	if(GPIO_Pin == plus) {
		InterruptHelper(plus, 4);
	}
	if(GPIO_Pin == minus) {
		InterruptHelper(minus, 5);
	}
}

void read_button(void){
	char msg [] = buttons;
	UART_print(msg);
}
