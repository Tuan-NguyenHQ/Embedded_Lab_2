/*
 * software_timer.h
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "tim.h"

extern uint16_t flag_timer[10];

void timer_init();
void setTimer(uint16_t duration, uint16_t index);

#endif /* INC_SOFTWARE_TIMER_H_ */
