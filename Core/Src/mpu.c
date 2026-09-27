/*
 * mpu.c
 *
 *  Created on: Jan 16, 2026
 *      Author: Kiran Duriseti
 */

#include "main.h"
#include "mpu.h"
#include <string.h>
#include <stdint.h>
#include "i2c.h"
#include "stm32f4xx_hal.h"

#define MPU_add (0x68 << 1) //logic low
#define mpu_timeout_ms 3U
#define ACK 0x75 //who am I
#define power 0x6B
#define power_2 0x6C
#define sample_rate 0x19
#define gyro_config 0x1B
#define acc_config 0x1C
#define multi_acc 0x3B
#define multi_gyro 0x43
#define acc_normalize 16384.0
#define gyro_normalize 131.0
#define config 0x1A

int32_t gyro_bias_x = 0;
int32_t gyro_bias_y = 0;

uint8_t data;
uint8_t check; //check = 0x68
static HAL_StatusTypeDef mpu_status = HAL_OK;
static uint8_t gyro_ready = 0;
static uint8_t awake_power = 0;

HAL_StatusTypeDef mpu_get_status(void) { return mpu_status; }
uint8_t mpu_gyro_ready(void) { return gyro_ready; }

extern I2C_HandleTypeDef hi2c2;

uint8_t mpu_read(uint8_t reg){
	data = 0;
	mpu_status = HAL_I2C_Mem_Read(&hi2c2, MPU_add, reg, 1, &data, 1, mpu_timeout_ms);
	return data;
}

void mpu_write(uint8_t reg, uint8_t d){
	mpu_status = HAL_I2C_Mem_Write(&hi2c2, MPU_add, reg, 1, &d, 1, mpu_timeout_ms);
}

void mpu_sleep(void) {
	//data = 64;
	//HAL_I2C_Mem_Write(&hi2c2, MPU_add, power, 1, &Data, 1, HAL_MAX_DELAY);
	mpu_write(power, awake_power | 0x40);
}

void mpu_wake(void) {
	//data = 0;
	//HAL_I2C_Mem_Write(&hi2c2, MPU_add, power, 1, &Data, 1, HAL_MAX_DELAY);
	mpu_write(power, awake_power);
}

HAL_StatusTypeDef mpu_read_gyro(int16_t *x, int16_t *y) {
	uint8_t rx[4];

	HAL_StatusTypeDef ok = HAL_I2C_Mem_Read(&hi2c2, MPU_add, multi_gyro, 1, rx, 4, mpu_timeout_ms);

	if (ok != HAL_OK) return ok;
	*x = (int16_t)(rx[0] << 8 | rx[1]);
	*y = (int16_t)(rx[2] << 8 | rx[3]);
	//*z = (int16_t)(rx[4] << 8 | rx[5]);
	return HAL_OK;
}

void mpu_calibrate_gyro(uint16_t samples)
{
    if (samples == 0) { mpu_status = HAL_ERROR; return; }
    int64_t sx = 0, sy = 0;
    for (uint16_t i = 0; i < samples; i++) {
        int16_t rx, ry;
        if (mpu_read_gyro(&rx, &ry) != HAL_OK) {
            mpu_status = HAL_ERROR;
            gyro_ready = 0;
            return;
        }

        sx += rx;
        sy += ry;
        HAL_Delay(2);
    }

    gyro_bias_x = (int32_t)(sx / samples);
    gyro_bias_y = (int32_t)(sy / samples);
    mpu_status = HAL_OK;
}

void mpu_read_acc(int16_t *x, int16_t *y, int16_t *z) {
	uint8_t rx[6];

	mpu_status = HAL_I2C_Mem_Read(&hi2c2, MPU_add, multi_acc, 1, rx, 6, mpu_timeout_ms);
    if (mpu_status != HAL_OK) return;

	int16_t raw_x, raw_y, raw_z;
	raw_x = (int16_t)(rx[0] << 8 | rx[1]);
	raw_y = (int16_t)(rx[2] << 8 | rx[3]);
	raw_z = (int16_t)(rx[4] << 8 | rx[5]);
	*x = raw_x;
	*y = raw_y;
	*z = raw_z;
}

void mpu_init_gyro(void){
    gyro_ready = 0;
    awake_power = 0x09;
	//only enables gyro readings (disable acc and temp to save power)
	//HAL_I2C_Mem_Read(&hi2c2, MPU_add, ACK, 1, &check, 1, HAL_MAX_DELAY);
	check = mpu_read(ACK);
    if (mpu_status != HAL_OK || check != 0x68) { mpu_status = HAL_ERROR; return; }
	//mpu_wake();
	mpu_write(power, 0x09); //disabled tempurature sensor
    if (mpu_status != HAL_OK) goto finish;
	HAL_Delay(50);

	mpu_write(power_2, 0x38); //diable acc
    if (mpu_status != HAL_OK) goto finish;

	//mpu_write(config, 0);
	mpu_write(config, 0x3);
    if (mpu_status != HAL_OK) goto finish;
	mpu_write(sample_rate, 0x0);
    if (mpu_status != HAL_OK) goto finish;

	//mpu_write(gyro_config, 0);
	mpu_write(gyro_config, 0x18);
    if (mpu_status != HAL_OK) goto finish;

	mpu_calibrate_gyro(20);
    gyro_ready = (mpu_status == HAL_OK);
finish:
    {
        HAL_StatusTypeDef initialization_status = mpu_status;
        mpu_sleep();
        if (initialization_status != HAL_OK) mpu_status = initialization_status;
        if (mpu_status != HAL_OK) gyro_ready = 0;
    }
}

void mpu_init(void){
    awake_power = 0;
    gyro_ready = 0;
	//HAL_I2C_Mem_Read(&hi2c2, MPU_add, ACK, 1, &check, 1, HAL_MAX_DELAY);
	check = mpu_read(ACK);
	mpu_wake();
	HAL_Delay(50);
	//data = 0; //8KHz
	//HAL_I2C_Mem_Write(&hi2c2, MPU_add, sample_rate, 1, &data, 1, HAL_MAX_DELAY);
	mpu_write(sample_rate, 0);
	//DLPF == 0, therfore 8KHz sample rate
	//data = 0; //+- 2g
	//HAL_I2C_Mem_Write(&hi2c2, MPU_add, acc_config, 1, &data, 1, HAL_MAX_DELAY);
	mpu_write(acc_config, 0);
	//data = 0; //+- 250 degrees/s
	//HAL_I2C_Mem_Write(&hi2c2, MPU_add, acc_config, 1, &data, 1, HAL_MAX_DELAY);
	mpu_write(gyro_config, 0);
}
