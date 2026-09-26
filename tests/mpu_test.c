/* Exercise the production sensor driver with failed and successful I2C reads. */
#define INC_MPU_H_
#define __MAIN_H
#define __I2C_H__
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
typedef enum { HAL_OK, HAL_ERROR } HAL_StatusTypeDef;
typedef struct { int unused; } I2C_HandleTypeDef;
#define I2C_MEMADD_SIZE_8BIT 1
I2C_HandleTypeDef hi2c2;
static bool wrong_identity, fail_writes;
static unsigned reads, fail_every, power_value, sample_divider, range_value, delays;
static HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *h, unsigned address,
    unsigned reg, unsigned size, uint8_t *out, unsigned count, unsigned timeout)
{
    (void)h; (void)size;
    assert(address == 0xd0 && timeout == 3);
    if (reg == 0x75) { out[0] = wrong_identity ? 0 : 0x68; return HAL_OK; }
    assert(reg == 0x43 && count == 4);
    if (fail_every && ++reads % fail_every == 0) return HAL_ERROR;
    out[0] = 0; out[1] = 164; out[2] = 0xfe; out[3] = 0xb8; /* +164, -328 */
    return HAL_OK;
}
static HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *h, unsigned address,
    unsigned reg, unsigned size, uint8_t *data, unsigned count, unsigned timeout)
{
    (void)h; (void)size;
    assert(address == 0xd0 && timeout == 3 && count == 1);
    if (reg == 0x6b) power_value = *data;
    if (reg == 0x19) sample_divider = *data;
    if (reg == 0x1b) range_value = *data;
    return fail_writes ? HAL_ERROR : HAL_OK;
}
static void HAL_Delay(unsigned ms) { delays += ms; }
#include "../Core/Src/mpu.c"
int main(void)
{
    wrong_identity = true; mpu_init_gyro(); assert(!mpu_gyro_ready());
    wrong_identity = false; fail_writes = true; mpu_init_gyro(); assert(!mpu_gyro_ready());
    fail_writes = false; fail_every = 0; delays = 0;
    mpu_init_gyro(); assert(mpu_gyro_ready() && mpu_get_status() == HAL_OK);
    assert(gyro_bias_x == 164 && gyro_bias_y == -328 && power_value == 0x09);
    assert(sample_divider == 0 && range_value == 0x18 && delays == 50 + 20*2);
    mpu_sleep(); assert(mpu_get_status() == HAL_OK && power_value == 0x49);
    mpu_wake(); assert(mpu_get_status() == HAL_OK && power_value == 0x09);
    fail_every = 1; reads = 0;
    int16_t x = 123, y = -456;
    assert(mpu_read_gyro(&x, &y) == HAL_ERROR && x == 123 && y == -456);
    mpu_init_gyro(); assert(!mpu_gyro_ready() && mpu_get_status() == HAL_ERROR);
    assert(reads == 2); /* A failed calibration aborts instead of using garbage. */
    assert(gyro_bias_x == 164 && gyro_bias_y == -328);
    fail_every = 0; mpu_init(); /* Original general initialization API still works. */
    assert(range_value == 0 && sample_divider == 0 && power_value == 0);
    puts("PASS: original gyro settings/calibration/API; failed reads/writes; sleep preserves power bits");
    return 0;
}
