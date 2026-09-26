#define INC_MPU_H_
#define __MAIN_H
#define __I2C_H__
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>

typedef enum { HAL_OK, HAL_ERROR, HAL_BUSY, HAL_TIMEOUT } HAL_StatusTypeDef;
typedef struct { int unused; } I2C_HandleTypeDef;
I2C_HandleTypeDef hi2c2;
static bool wrong_identity;
static unsigned reads, fail_at, power_value;
static HAL_StatusTypeDef read_error = HAL_OK, write_error = HAL_OK;
static const int16_t samples_x[] = {100, 200, -300};
static const int16_t samples_y[] = {-100, -200, 600};

static HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *h, unsigned address,
    unsigned reg, unsigned size, uint8_t *out, unsigned count, unsigned timeout)
{
    (void)h;
    assert(address == 0xd0 && size == 1 && timeout > 0 && timeout <= 10);
    if (reg == 0x75) {
        if (read_error != HAL_OK && fail_at == 0) return read_error;
        assert(count == 1);
        out[0] = wrong_identity ? 0 : 0x68;
        return HAL_OK;
    }
    assert(reg == 0x43 && count == 4);
    unsigned index = reads++ % 3;
    if (read_error != HAL_OK && (fail_at == 0 || reads == fail_at)) return read_error;
    uint16_t x = (uint16_t)samples_x[index], y = (uint16_t)samples_y[index];
    out[0] = (uint8_t)(x >> 8); out[1] = (uint8_t)x;
    out[2] = (uint8_t)(y >> 8); out[3] = (uint8_t)y;
    return HAL_OK;
}
static HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *h, unsigned address,
    unsigned reg, unsigned size, uint8_t *data, unsigned count, unsigned timeout)
{
    (void)h;
    assert(address == 0xd0 && size == 1 && count == 1 && timeout > 0 && timeout <= 10);
    if (write_error != HAL_OK) return write_error;
    if (reg == 0x6b) power_value = *data;
    return HAL_OK;
}
static void HAL_Delay(unsigned ms) { (void)ms; }
#include "../Core/Src/mpu.c"

int main(void)
{
    wrong_identity = true;
    mpu_init_gyro(); assert(!mpu_gyro_ready() && mpu_get_status() != HAL_OK);
    wrong_identity = false;
    read_error = HAL_TIMEOUT;
    mpu_init_gyro(); assert(!mpu_gyro_ready());
    read_error = HAL_OK; write_error = HAL_ERROR;
    mpu_init_gyro(); assert(!mpu_gyro_ready());
    write_error = HAL_OK;
    mpu_init_gyro(); assert(mpu_gyro_ready() && mpu_get_status() == HAL_OK);

    reads = 0;
    mpu_calibrate_gyro(3);
    assert(gyro_bias_x == 0 && gyro_bias_y == 100 && mpu_get_status() == HAL_OK);
    reads = 0; read_error = HAL_TIMEOUT; fail_at = 2;
    mpu_calibrate_gyro(3);
    assert(reads == 2 && !mpu_gyro_ready() && mpu_get_status() != HAL_OK);
    assert(gyro_bias_x == 0 && gyro_bias_y == 100);
    mpu_calibrate_gyro(0); assert(mpu_get_status() != HAL_OK);

    fail_at = 0;
    int16_t x = 123, y = -456;
    assert(mpu_read_gyro(&x, &y) == HAL_TIMEOUT && x == 123 && y == -456);
    read_error = HAL_OK; reads = 2;
    assert(mpu_read_gyro(&x, &y) == HAL_OK && x == -300 && y == 600);
    mpu_init_gyro(); assert(mpu_gyro_ready());

    unsigned awake = power_value;
    assert((awake & 0x40U) == 0);
    mpu_sleep(); assert(power_value == (awake | 0x40U));
    mpu_wake(); assert(power_value == awake);
    write_error = HAL_TIMEOUT;
    mpu_sleep(); assert(mpu_get_status() == HAL_TIMEOUT && power_value == awake);
    write_error = HAL_OK;
    mpu_sleep(); assert(mpu_get_status() == HAL_OK && power_value == (awake | 0x40U));
    puts("PASS: MPU initialization failures, signed samples, calibration averaging/abort, read recovery, sleep/wake errors");
    return 0;
}
