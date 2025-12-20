/*
 * UART_print.c
 *
 *  Created on: Dec 20, 2025
 *      Author: Kiran Duriseti
 */
#include "main_loop.h"
#include "usart.h"
#include "gpio.h"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart2;

void UART_print(char* msg){
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}
