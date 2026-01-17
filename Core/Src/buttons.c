/*
 * buttons.c
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#include <string.h>
#include <stdio.h>
#include "stm32f4xx_hal.h"
#include "joystick.h"
#include "gpio.h"
#include "accelerometer.h"
#include "mpu.h"

//ACC_control //
//CS //
typedef enum {
    A = 0, //
    B,//
    X,//
    Y,//
    PLUS,//
    MINUS,//
    ZR,//
    ZL,//
    R,//
    L,//
    HOME,//
    CAPTURE,//
    JL,//
    JR,//
	BTN_COUNT
} ButtonName;

//ABXY+-
#define num_buttons BTN_COUNT
#define debounce 10

volatile int buttons [num_buttons] = {0};
int acc_state = 1;
int last_acc = 0;

uint8_t  last_raw[num_buttons] = {0};
uint32_t last_change_ms[num_buttons] = {0};

GPIO_TypeDef* ports[num_buttons] = {
  A_GPIO_Port, B_GPIO_Port, X_GPIO_Port, Y_GPIO_Port, plus_GPIO_Port,
  minus_GPIO_Port, Zr_GPIO_Port, Zl_GPIO_Port, r_GPIO_Port, l_GPIO_Port,
  home_GPIO_Port, capture_GPIO_Port, jl_GPIO_Port, jr_GPIO_Port,
};

uint16_t pins[num_buttons] = {
  A_Pin, B_Pin, X_Pin, Y_Pin, plus_Pin, minus_Pin, Zr_Pin, Zl_Pin, r_Pin, l_Pin,
  home_Pin, capture_Pin, jl_Pin, jr_Pin,
};

uint8_t raw_pressed(uint8_t i) {
    return (HAL_GPIO_ReadPin(ports[i], pins[i]) == GPIO_PIN_RESET) ? 1 : 0;
}

int get_acc_state(void){
	return acc_state;
}

void ACC_power(void){
	if (acc_state == 1) {
		//StandBy();
		mpu_sleep();

	}
	else {
		//Wake();
		mpu_wake();
	}
}

void buttons_update(void) {
    uint32_t now = HAL_GetTick();
    ACC_power();
    for (uint8_t i = 0; i < num_buttons; i++) {
        uint8_t raw = raw_pressed(i);

        // if raw input changed, restart stability timer
        if (raw != last_raw[i]) {
            last_raw[i] = raw;
            last_change_ms[i] = now;
        }

        // if raw has been stable long enough, commit it
        if ((now - last_change_ms[i]) >= debounce) {
            buttons[i] = raw;   // 1 while held, 0 when released
        }
    }
}

void buttons_print(void){
	char msg[64];
	snprintf(msg, sizeof(msg),
			 "A:%d B:%d X:%d Y:%d +:%d -:%d\r\n",
			 buttons[0], buttons[1], buttons[2],
			 buttons[3], buttons[4], buttons[5]);
	printf(msg);
}


//A, B, X, Y, plus, minus, Zr, Zl, r, l, home, capture, jl, jr : array order
//A, B, X, Y, l, r, plus, minus, Zr, Zl, home, capture, jl, jr : report order
//A 0->0, B 1->1, X 2->2, Y 3->3, l 9->4, 8->5, plus, minus, Zr, Zl, home, capture, jl, jr : report order

//for x-box controller
uint16_t get_report_buttons(void) {
	uint16_t send = (uint16_t)((buttons[A] << 0) |
						 	 (buttons[B] << 1) |
							 (buttons[X] << 2) |
							 (buttons[Y] << 3) |
							 (buttons[L] << 4) |
							 (buttons[R] << 5) |
							 (buttons[ZL] << 6) |
							 (buttons[ZR] << 7) |
							 (buttons[MINUS] << 8) |
							 (buttons[PLUS] << 9) |
							 (buttons[JL] << 10) |
							 (buttons[JR] << 11) |
							 (buttons[HOME] << 12) | //up on d-pad
							 (buttons[CAPTURE] << 13) ); //down on d-pad


	return send;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN) {
	if (GPIO_PIN == ACC_control_Pin) {
		int now = HAL_GetTick();
		//HAL_GPIO_TogglePin(ACC_control_GPIO_Port, ACC_control_Pin);
		if (now - last_acc > debounce){
			last_acc = now;
			acc_state ^= 1;
		}
	}
}
