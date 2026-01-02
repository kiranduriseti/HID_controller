/*
 * buttons.h
 *
 *  Created on: Dec 21, 2025
 *      Author: Kiran Duriseti
 */

#ifndef INC_BUTTONS_H_
#define INC_BUTTONS_H_

void buttons_update(void);
void buttons_print(void);
uint8_t get_report_buttons(void);
int get_acc_state(void);

#endif /* INC_BUTTONS_H_ */
