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

void main_loop(void){
	char msg[] = "Hello World\r\n";
	UART_print(msg);
	HAL_Delay(10);
}

