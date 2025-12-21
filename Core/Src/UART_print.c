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

#define IO_SIZE 256
#define BufferSize 32

static volatile char inputs[IO_SIZE];
static volatile uint8_t input_size = 0;

static volatile uint8_t tx_buffer1[IO_SIZE];
static volatile uint8_t tx_buffer2[IO_SIZE];

static volatile uint8_t *active = tx_buffer1;
static volatile uint8_t *pending = tx_buffer2;
static volatile uint32_t pending_size = 0;

static volatile uint8_t tx_busy = 0;

void UART_print_blocking(char* msg){
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
	HAL_Delay(1000);
}

void UART_print(char *msg){
	if (msg == NULL) return;

	__disable_irq();

	if (!tx_busy){
		sprintf((char*)active, "%s", msg);

		tx_busy = 1;

		__enable_irq();

		HAL_UART_Transmit_DMA(&huart2, (uint8_t*)active, strlen(active));

//		if (HAL_UART_Transmit_DMA(&huart2, (uint8_t*)active, strlen(active)) != HAL_OK){
//			tx_busy = 0;
//		}
	}
	else {
		sprintf((char*)pending, "%s", msg);

		pending_size = strlen(msg);

		__enable_irq();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
	if (huart->Instance != USART2) return;

	__disable_irq();

	if (pending_size == 0) {
		tx_busy = 0;
		__enable_irq();
		return;
	}

	uint8_t *tmp = active;
	active = pending;
	pending = tmp;

	uint16_t len = pending_size;
	pending_size = 0;

	__enable_irq();

	if (HAL_UART_GetState(huart) == HAL_UART_STATE_READY) {
		HAL_UART_Transmit_DMA(&huart2, (uint8_t*)active, len);
	}
}
