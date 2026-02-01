/*
 * fan_control.h
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#ifndef CORE_INC_FAN_CONTROL_H_
#define CORE_INC_FAN_CONTROL_H_

#include <stdint.h>
#include "tim.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint16_t current_speed;
} FanHandle_t;

void Fan_Init(FanHandle_t *hfan,
              TIM_HandleTypeDef *htim,
              uint32_t channel);

void Fan_Start(FanHandle_t *hfan);
void Fan_Stop(FanHandle_t *hfan);
void Fan_SetSpeed(FanHandle_t *hfan, uint16_t speed);
uint16_t Fan_GetSpeed(FanHandle_t *hfan);
void Fan_Off(FanHandle_t *hfan);

#endif /* CORE_INC_FAN_CONTROL_H_ */
