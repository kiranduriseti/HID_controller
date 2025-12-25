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
#include "usb_device.h"

#include "usbd_customhid.h"

extern USBD_HandleTypeDef hUsbDeviceFS;

void print_loop () {
	//if (fresh_data) {
//	read_button();
//	joystick_print();
	//}
}

void send_report(){
	joystick_report report = get_report();
	USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&report, 5);
	HAL_Delay(5);
}

void main_loop(void){
	send_report();

	//UART_print("Hello World\r\n");
	//UART_print("Testing Buffer\r\n");

//	buttons_update();
//	joystick_update();
//
//	read_button();
//	HAL_Delay(10);
//	joystick_print();
//	HAL_Delay(4000);
}

