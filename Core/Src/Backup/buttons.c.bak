/*
 * buttons.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include "UART_print.h"
#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"

//ABXY+-
#define num_buttons 6
#define debounce 10

#define A GPIO_PIN_3
#define B GPIO_PIN_4
#define X GPIO_PIN_5
#define Y GPIO_PIN_6
#define plus GPIO_PIN_8
#define minus GPIO_PIN_9

uint32_t pins[num_buttons] = {A, B, X, Y, plus, minus};
int buttons [num_buttons] = {0};

uint32_t pending_time [num_buttons] = {0};
uint32_t pending[num_buttons] = {0};

//int state = 0;
//void InterruptHelper(uint16_t GPIO_PIN, uint8_t index){
//	uint32_t now = HAL_GetTick();
//
//	if (now - debounce_time[index] > debounce){
//		uint8_t level = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN);
//		buttons[index] = (level == 0) ? 1 : 0;
//
//		debounce_time[index] = now;
//	}
//}

void buttons_update(void){
	uint32_t now = HAL_GetTick();

	for (uint8_t i = 0; i < num_buttons; i++) {
		if (!pending[i]) continue;

		if ((now - pending_time[i]) < debounce) continue;

		int state = HAL_GPIO_ReadPin(GPIOB, pins[i]);
		buttons[i] = (state == 0) ? 1 : 0;

		pending[i] = 0;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	uint32_t now = HAL_GetTick();

	for (int i = 0; i < num_buttons; i++) {
		if (GPIO_Pin == pins[i]){
			pending[i] = 1;
			pending_time[i] = now;
			break;
		}
	}

}

void read_button(void){
	char msg[64];
	snprintf(msg, sizeof(msg),
			 "A:%d B:%d X:%d Y:%d +:%d -:%d\r\n",
			 buttons[0], buttons[1], buttons[2],
			 buttons[3], buttons[4], buttons[5]);
	UART_print(msg);
}
