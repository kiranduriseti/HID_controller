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
#include "joystick.h"
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

void buttons_update(void){
	uint32_t now = HAL_GetTick();

	for (uint8_t i = 0; i < num_buttons; i++) {
		if (!pending[i]) continue;

		if ((now - pending_time[i]) < debounce) continue;

		int state = HAL_GPIO_ReadPin(GPIOB, pins[i]);
		buttons[i] = (state == 0) ? 1 : 0;

		pending[i] = 0;
		//fresh_data = 1;
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

void buttons_print(void){
	//fresh_data = 0;
	char msg[64];
	snprintf(msg, sizeof(msg),
			 "A:%d B:%d X:%d Y:%d +:%d -:%d\r\n",
			 buttons[0], buttons[1], buttons[2],
			 buttons[3], buttons[4], buttons[5]);
	UART_print(msg);
}

uint8_t get_report_buttons(void) {
	uint8_t send = (uint8_t) ((buttons[0] << 6) |
						 (buttons[1] << 5) |
						 (buttons[2] << 4) |
						 (buttons[3] << 3) |
						 (buttons[4] << 2) |
						 (buttons[5] << 1)
	);

	return send;
}
