/*
 * accelerometer.h
 *
 *  Created on: Jan 1, 2026
 *      Author: Kiran Duriseti
 */

#ifndef INC_ACCELEROMETER_H_
#define INC_ACCELEROMETER_H_

void readValues(double* x, double* y, double* z);
void initACC(void);
void StandBy(void);
void Wake(void);

#endif /* INC_ACCELEROMETER_H_ */
