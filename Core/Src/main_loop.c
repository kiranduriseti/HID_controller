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

void main_loop(void){
	//UART_print_blocking ("Test\r\n");
	char msg[] = "Hello World\r\n";
	UART_print(msg);
	HAL_Delay(1000);

	UART_print("Testing Buffer\r\n");
	HAL_Delay(1000);

	buttons_update();
}

