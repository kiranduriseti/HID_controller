#ifdef _MSC_VER
#define __attribute__(x)
#pragma warning(disable: 4244 4459 4310) /* Original code, combined translation units, explicit mask narrowing. */
#endif
#define __GPIO_H__
#define __ADC_H__
#define __TIM_H__
#define __I2C_H__
#define __USB_DEVICE__H__
#define __USB_CUSTOMHID_H
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
typedef struct { unsigned id; } GPIO_TypeDef;
static GPIO_TypeDef port_a = {0}, port_b = {1}, port_c = {2};
#define GPIOA (&port_a)
#define GPIOB (&port_b)
#define GPIOC (&port_c)
#define GPIO_PIN_RESET 0
#define GPIO_PIN_0 (1U << 0)
#define GPIO_PIN_1 (1U << 1)
#define GPIO_PIN_2 (1U << 2)
#define GPIO_PIN_3 (1U << 3)
#define GPIO_PIN_4 (1U << 4)
#define GPIO_PIN_5 (1U << 5)
#define GPIO_PIN_6 (1U << 6)
#define GPIO_PIN_7 (1U << 7)
#define GPIO_PIN_8 (1U << 8)
#define GPIO_PIN_9 (1U << 9)
#define GPIO_PIN_10 (1U << 10)
#define GPIO_PIN_11 (1U << 11)
#define GPIO_PIN_12 (1U << 12)
#define GPIO_PIN_13 (1U << 13)
typedef struct { unsigned Instance; } ADC_HandleTypeDef;
typedef struct { int unused; } TIM_HandleTypeDef;
typedef struct { int unused; } USBD_HandleTypeDef;
#define ADC1 1
static ADC_HandleTypeDef hadc1 = {ADC1};
static TIM_HandleTypeDef htim3;
static uint32_t tick, irq_mask;
static uint16_t pressed[3];
static unsigned sleeps, wakes;
static int16_t sensor_x, sensor_y;
static bool sensor_fail;
int32_t gyro_bias_x, gyro_bias_y;
static uint32_t HAL_GetTick(void) { return tick; }
static void HAL_Delay(uint32_t ms) { tick += ms; }
static unsigned HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin) { return !(pressed[port->id] & pin); }
static uint32_t __get_PRIMASK(void) { return irq_mask; }
static void __disable_irq(void) { irq_mask = 1; }
static void __set_PRIMASK(uint32_t mask) { irq_mask = mask; }
static HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef *a, uint32_t *buf, unsigned n) { (void)a; (void)buf; (void)n; return HAL_OK; }
static HAL_StatusTypeDef HAL_TIM_Base_Start(TIM_HandleTypeDef *t) { (void)t; return HAL_OK; }
#include "main.h"
void Error_Handler(void) { abort(); }
void mpu_sleep(void) { ++sleeps; }
void mpu_wake(void) { ++wakes; }
uint8_t mpu_gyro_ready(void) { return 1; }
HAL_StatusTypeDef mpu_get_status(void) { return HAL_OK; }
HAL_StatusTypeDef mpu_read_gyro(int16_t *x, int16_t *y) {
    if (sensor_fail) return HAL_ERROR;
    *x = sensor_x; *y = sensor_y; return HAL_OK;
}
#include "../Core/Src/buttons.c"
#include "../Core/Src/joystick.c"
#include "../Core/Src/main_loop.c"
#include "original_behavior.h"
void controller_usb_toggle(uint32_t now) { (void)now; }
void controller_usb_poll(uint32_t now) { (void)now; }
uint8_t controller_usb_send(const joystick_report *s, uint32_t now) { (void)s; (void)now; return 0; }
int main(void) {
    for (int32_t value = -ADC_max; value <= ADC_max; ++value)
        assert(deadzone_scale(value) == original_deadzone_scale(value));
    for (uint32_t mask = 0; mask < (1U << BTN_COUNT); ++mask) {
        for (unsigned b = 0; b < BTN_COUNT; ++b) buttons[b] = (mask >> b) & 1U;
        assert(get_report_buttons() == original_get_report_buttons());
    }
    memset((void *)buttons, 0, sizeof(buttons));
    for (unsigned b = 0; b < BTN_COUNT; ++b) {
        tick += 20; pressed[ports[b]->id] |= pins[b]; buttons_update();
        tick += 9; buttons_update(); assert(buttons[b] == 0);
        tick += 1; buttons_update(); assert(buttons[b] == 1);
        pressed[ports[b]->id] &= (uint16_t)~pins[b]; buttons_update();
        tick += 10; buttons_update(); assert(buttons[b] == 0);
    }
    assert(sleeps == 1 && wakes == 0);
    /* Press bounce, stable press, hold, release bounce and second press. */
    pressed[2] |= ACC_control_Pin; buttons_update();
    tick += 5; pressed[2] &= (uint16_t)~ACC_control_Pin; buttons_update();
    tick += 5; pressed[2] |= ACC_control_Pin; buttons_update();
    tick += 9; buttons_update(); assert(get_acc_state() == 1);
    tick += 1; buttons_update(); assert(get_acc_state() == 0 && wakes == 1);
    tick += 1000; buttons_update(); assert(wakes == 1);
    pressed[2] &= (uint16_t)~ACC_control_Pin; buttons_update();
    tick += 5; pressed[2] |= ACC_control_Pin; buttons_update();
    tick += 20; buttons_update(); assert(wakes == 1);
    pressed[2] &= (uint16_t)~ACC_control_Pin; buttons_update();
    tick += 10; buttons_update(); assert(get_acc_state() == 0);
    tick = UINT32_MAX - 5;
    pressed[2] |= ACC_control_Pin; buttons_update();
    tick += 10; buttons_update();
    assert(get_acc_state() == 1 && sleeps == 2);
    for (unsigned ch = 0; ch < channels; ++ch) joystick_adc[ch] = (uint16_t)(ADC_center + ch * 10);
    uint32_t before = tick; joystick_calibrate(20); assert(tick - before == 40);
    joystick_update(); joystick_report state = get_report();
    assert(state.lx == 0 && state.ly == 0 && state.rx == 0 && state.ry == 0);
    joystick_adc[0] = ADC_max; joystick_adc[1] = 0;
    joystick_update(); state = get_report(); assert(state.lx == clamp && state.ly == -clamp);
    irq_mask = 1; (void)get_report(); assert(irq_mask == 1); irq_mask = 0;
    uint32_t random = 123;
    for (unsigned i = 0; i < 10000; ++i) {
        random = random * 1664525U + 1013904223U; sensor_x = (int16_t)random;
        random = random * 1664525U + 1013904223U; sensor_y = (int16_t)random;
        original_mpu_joy(); int16_t expected_x = gyro_x, expected_y = gyro_y;
        mpu_joy(); assert(gyro_x == expected_x && gyro_y == expected_y);
    }
    sensor_fail = true; mpu_joy(); assert(gyro_x == 0 && gyro_y == 0);
    acc_state = 0; gyro_x = clamp; gyro_y = -clamp;
    state.rx = 100; state.ry = -100; state = Jr_update(state);
    assert(state.rx == clamp && state.ry == -clamp);
    acc_state = 1; state.rx = 100; state.ry = -100; state = Jr_update(state);
    assert(state.rx == 100 && state.ry == -100);
    puts("PASS: original button mapping, debounce/polled MPU toggle, stick curve, calibration, 10000 gyro samples and motion toggle");
    return 0;
}
