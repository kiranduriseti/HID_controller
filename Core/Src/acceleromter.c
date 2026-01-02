/*
 * acceleromter.c
 *
 *  Created on: Jan 1, 2026
 *      Author: Kiran Duriseti
 */

#include "main.h"
#include <string.h>
#include <stdint.h>
#include "spi.h"

//PC1, MOSI
//PC2, MISO
//PB10, SPI2 clk

#define READ 0x80 //Bit 7 = 1 for read
#define WRITE 0x00 //Bit 7 = 0
#define MultiByte 0x40 //bit 6 = 1 for MultiByte

#define BW_Rate 0x2C
#define Power_Ctrl 0x2D
#define Data_Format 0x31
#define X0 0x32
#define X1 0x33
#define Y0 0x34
#define Y1 0x35
#define Z0 0x36
#define Z1 0x37

#define Rescale 0.0039 //3.9mg/LSB

void CS_LOW (void) {
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, 0);
}

void CS_HIGH (void) {
	HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, 1);
}

void accWrite(uint8_t addr, uint8_t val){
	uint8_t tx[2];
	tx[0] = addr | WRITE;
	tx[1] = val;

	CS_LOW();
	HAL_SPI_Transmit(&hspi2, tx, 2, HAL_MAX_DELAY);
	CS_HIGH();
}

uint8_t accRead(uint8_t addr) {
	uint8_t tx[2];
	uint8_t rx[2];

	tx[0] = addr | READ;
	tx[1] = 0x00;

	CS_LOW();
	HAL_SPI_TransmitReceive(&hspi2, tx, rx, 2, HAL_MAX_DELAY);
	CS_HIGH();

	return rx[1];
}

void readValues(double* x, double* y, double* z)
{
    uint8_t tx[7]; //command + 6 data bytes
    uint8_t rx[7];

    // start at DATAX0 (0x32), READ + MULTI
    tx[0] = X0 | READ | MultiByte;  // READ | MULTI //byte 0
    memset(&tx[1], 0, 6);

    CS_LOW();
    HAL_SPI_TransmitReceive(&hspi2, tx, rx, 7, HAL_MAX_DELAY);
    CS_HIGH();

    int16_t rawX = (int16_t)((rx[2] << 8) | rx[1]);
    int16_t rawY = (int16_t)((rx[4] << 8) | rx[3]);
    int16_t rawZ = (int16_t)((rx[6] << 8) | rx[5]);

    *x = Rescale * rawX;
    *y = Rescale * rawY;
    *z = Rescale * rawZ;
}


void StandBy(void) {
	accWrite(Power_Ctrl, 0x00);
}

void Wake(void) {
	accWrite(Power_Ctrl, 0x08);
}

void initACC(void) {
	accWrite(BW_Rate, 0x0A); //A = 1010 for 100Hz ODR
	// set full range mode
	accWrite(Data_Format, 0x08); //sets full resolution bit D3: 0000 1000
	// enable measurement
	//accWrite(Power_Ctrl, 0x08); //sets measure bit D3: 0000 1000
	Wake();

	CS_HIGH();
}
