/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI_MOSI_Pin GPIO_PIN_1
#define SPI_MOSI_GPIO_Port GPIOC
#define SPI_MISO_Pin GPIO_PIN_2
#define SPI_MISO_GPIO_Port GPIOC
#define lx_Pin GPIO_PIN_0
#define lx_GPIO_Port GPIOA
#define ly_Pin GPIO_PIN_1
#define ly_GPIO_Port GPIOA
#define rx_Pin GPIO_PIN_4
#define rx_GPIO_Port GPIOA
#define ry_Pin GPIO_PIN_5
#define ry_GPIO_Port GPIOA
#define CS_Pin GPIO_PIN_6
#define CS_GPIO_Port GPIOA
#define SPI_CLK_Pin GPIO_PIN_10
#define SPI_CLK_GPIO_Port GPIOB
#define ACC_control_Pin GPIO_PIN_14
#define ACC_control_GPIO_Port GPIOB
#define ACC_control_EXTI_IRQn EXTI15_10_IRQn
#define l_Pin GPIO_PIN_15
#define l_GPIO_Port GPIOB
#define home_Pin GPIO_PIN_6
#define home_GPIO_Port GPIOC
#define capture_Pin GPIO_PIN_7
#define capture_GPIO_Port GPIOC
#define Zr_Pin GPIO_PIN_8
#define Zr_GPIO_Port GPIOC
#define Zl_Pin GPIO_PIN_9
#define Zl_GPIO_Port GPIOC
#define jr_Pin GPIO_PIN_8
#define jr_GPIO_Port GPIOA
#define jl_Pin GPIO_PIN_9
#define jl_GPIO_Port GPIOA
#define r_Pin GPIO_PIN_10
#define r_GPIO_Port GPIOA
#define A_Pin GPIO_PIN_3
#define A_GPIO_Port GPIOB
#define B_Pin GPIO_PIN_4
#define B_GPIO_Port GPIOB
#define X_Pin GPIO_PIN_5
#define X_GPIO_Port GPIOB
#define Y_Pin GPIO_PIN_6
#define Y_GPIO_Port GPIOB
#define plus_Pin GPIO_PIN_8
#define plus_GPIO_Port GPIOB
#define minus_Pin GPIO_PIN_9
#define minus_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
