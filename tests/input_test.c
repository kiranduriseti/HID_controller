#ifdef _MSC_VER
#define __attribute__(x)
#pragma warning(disable: 4244 4459 4310)
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
static joystick_report sent;
void controller_usb_toggle(uint32_t now) { (void)now; }
void controller_usb_poll(uint32_t now) { (void)now; }
uint8_t controller_usb_send(const joystick_report *s, uint32_t now) { (void)now; sent = *s; return 0; }

static void sample_for(unsigned ms)
{
    for (unsigned i = 0; i < ms; ++i) { ++tick; buttons_update(); }
}
static void set_pin(GPIO_TypeDef *port, uint16_t pin, bool down)
{
    if (down) pressed[port->id] |= pin;
    else pressed[port->id] &= (uint16_t)~pin;
    buttons_update();
}
static void test_buttons(void)
{
    const struct { GPIO_TypeDef *port; uint16_t pin; unsigned bit; } mapping[] = {
        {A_GPIO_Port, A_Pin, 0}, {B_GPIO_Port, B_Pin, 1},
        {X_GPIO_Port, X_Pin, 2}, {Y_GPIO_Port, Y_Pin, 3},
        {l_GPIO_Port, l_Pin, 4}, {r_GPIO_Port, r_Pin, 5},
        {Zl_GPIO_Port, Zl_Pin, 6}, {Zr_GPIO_Port, Zr_Pin, 7},
        {minus_GPIO_Port, minus_Pin, 8}, {plus_GPIO_Port, plus_Pin, 9},
        {jl_GPIO_Port, jl_Pin, 10}, {jr_GPIO_Port, jr_Pin, 11},
        {home_GPIO_Port, home_Pin, 12}, {capture_GPIO_Port, capture_Pin, 13}
    };
    sample_for(20);
    for (unsigned i = 0; i < sizeof(mapping)/sizeof(mapping[0]); ++i) {
        GPIO_TypeDef *port = mapping[i].port;
        uint16_t pin = mapping[i].pin, bit = (uint16_t)(1U << mapping[i].bit);
        set_pin(port, pin, true); sample_for(9); assert(get_report_buttons() == 0);
        set_pin(port, pin, false); sample_for(15); assert(get_report_buttons() == 0);
        set_pin(port, pin, true); sample_for(3);
        set_pin(port, pin, false); sample_for(2);
        set_pin(port, pin, true); sample_for(9); assert(get_report_buttons() == 0);
        sample_for(1); assert(get_report_buttons() == bit);
        sample_for(100); assert(get_report_buttons() == bit);
        set_pin(port, pin, false); sample_for(9); assert(get_report_buttons() == bit);
        set_pin(port, pin, true); sample_for(3); assert(get_report_buttons() == bit);
        set_pin(port, pin, false); sample_for(10); assert(get_report_buttons() == 0);
    }
    set_pin(A_GPIO_Port, A_Pin, true);
    set_pin(B_GPIO_Port, B_Pin, true); sample_for(10);
    assert(get_report_buttons() == 3);
    set_pin(A_GPIO_Port, A_Pin, false); sample_for(10); assert(get_report_buttons() == 2);
    set_pin(B_GPIO_Port, B_Pin, false); sample_for(10); assert(get_report_buttons() == 0);
    tick = UINT32_MAX - 5U;
    set_pin(A_GPIO_Port, A_Pin, true); sample_for(10); assert(get_report_buttons() == 1);
    set_pin(A_GPIO_Port, A_Pin, false); sample_for(10); assert(get_report_buttons() == 0);

    assert(get_acc_state() == 1 && wakes == 0 && sleeps == 0);
    set_pin(ACC_control_GPIO_Port, ACC_control_Pin, true); sample_for(9);
    assert(get_acc_state() == 1);
    sample_for(1); assert(get_acc_state() == 0 && wakes == 1);
    sample_for(100); assert(get_acc_state() == 0 && wakes == 1);
    set_pin(ACC_control_GPIO_Port, ACC_control_Pin, false); sample_for(5);
    set_pin(ACC_control_GPIO_Port, ACC_control_Pin, true); sample_for(20);
    assert(get_acc_state() == 0 && wakes == 1);
    set_pin(ACC_control_GPIO_Port, ACC_control_Pin, false); sample_for(10);
    set_pin(ACC_control_GPIO_Port, ACC_control_Pin, true); sample_for(10);
    assert(get_acc_state() == 1 && sleeps == 1 && wakes == 1);
    sample_for(100);
    assert(get_acc_state() == 1 && sleeps == 1 && wakes == 1);
    set_pin(ACC_control_GPIO_Port, ACC_control_Pin, false); sample_for(10);
    assert(sleeps == 1 && wakes == 1);
    puts("PASS: all button mappings, short presses, press/release bounce, held/simultaneous buttons, tick wrap, gyro toggle rearm");
}
static void test_mode_gesture(void)
{
    mode_gesture g = {0};
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 100));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 2099));
    assert(mode_gesture_update(&g, MODE_BUTTONS, 2100));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 5000));
    assert(!mode_gesture_update(&g, 1U << 8, 5001));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 8000));
    assert(!mode_gesture_update(&g, 0, 8001));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 8002));
    assert(!mode_gesture_update(&g, 1U << 9, 9000));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 9001));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, 11000));
    assert(mode_gesture_update(&g, MODE_BUTTONS, 11001));
    g = (mode_gesture){0};
    uint32_t start = UINT32_MAX - 999U;
    assert(!mode_gesture_update(&g, MODE_BUTTONS, start));
    assert(!mode_gesture_update(&g, MODE_BUTTONS, start + 1999U));
    assert(mode_gesture_update(&g, MODE_BUTTONS, start + 2000U));

    set_pin(plus_GPIO_Port, plus_Pin, true); sample_for(10);
    send_report(); assert(sent.buttons == (1U << 9));
    set_pin(minus_GPIO_Port, minus_Pin, true); sample_for(10);
    set_pin(A_GPIO_Port, A_Pin, true); sample_for(10);
    send_report(); assert(sent.buttons == 1);
    mode_hold.latched = true;
    set_pin(minus_GPIO_Port, minus_Pin, false); sample_for(10);
    send_report(); assert(sent.buttons == 1);
    set_pin(plus_GPIO_Port, plus_Pin, false); sample_for(10);
    mode_gesture_update(&mode_hold, get_report_buttons(), tick);
    assert(!mode_hold.latched);
    set_pin(A_GPIO_Port, A_Pin, false); sample_for(10);
    puts("PASS: mode hold threshold, interrupted hold, release/rearm, tick wrap and outgoing chord suppression");
}
static void test_joystick(void)
{
    int previous = -clamp;
    for (int value = -ADC_max; value <= ADC_max; ++value) {
        int output = deadzone_scale(value);
        assert(output >= -clamp && output <= clamp && output >= previous);
        if (value > -deadzone && value < deadzone) assert(output == 0);
        if (value < 0) assert(output <= 0);
        if (value > 0) assert(output >= 0);
        previous = output;
    }
    assert(deadzone_scale(-ADC_center) == -clamp);
    assert(deadzone_scale(ADC_max - ADC_center) == clamp);
    assert(deadzone_scale(-300) < 0 && deadzone_scale(300) > 0);
    for (unsigned ch = 0; ch < channels; ++ch) joystick_adc[ch] = ADC_center;
    joystick_calibrate(20);
    for (unsigned ch = 0; ch < channels; ++ch) {
        for (unsigned high = 0; high < 2; ++high) {
            joystick_adc[ch] = high ? ADC_max : 0;
            joystick_update();
            joystick_report s = get_report();
            int values[] = {s.lx, s.ly, s.rx, s.ry};
            for (unsigned axis = 0; axis < 4; ++axis)
                assert(values[axis] == (axis == ch ? (high ? clamp : -clamp) : 0));
        }
        joystick_adc[ch] = ADC_center;
    }
    const uint16_t centers[] = {2010, 2090, 1980, 2120};
    for (unsigned ch = 0; ch < channels; ++ch) joystick_adc[ch] = centers[ch];
    joystick_calibrate(20); joystick_update();
    joystick_report s = get_report();
    assert(s.lx == 0 && s.ly == 0 && s.rx == 0 && s.ry == 0);
    joystick_calibrate(0); joystick_update(); s = get_report();
    assert(s.lx == 0 && s.ly == 0 && s.rx == 0 && s.ry == 0);
    irq_mask = 1; (void)get_report(); assert(irq_mask == 1); irq_mask = 0;
    puts("PASS: joystick deadzone, monotonicity, sign, limits/endpoints, four-axis mapping and offset calibration");
}
int main(void)
{
    test_buttons();
    test_mode_gesture();
    test_joystick();
    return 0;
}
