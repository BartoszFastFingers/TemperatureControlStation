/*
 * heater_control.h
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#ifndef CORE_INC_HEATER_CONTROL_H_
#define CORE_INC_HEATER_CONTROL_H_

#include <stdint.h>
#include "tim.h"

typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint16_t current_power;  // 0-999
} HeaterHandle_t;

void Heater_Init(HeaterHandle_t *hheater,
                 TIM_HandleTypeDef *htim,
                 uint32_t channel);

void Heater_Start(HeaterHandle_t *hheater);
void Heater_Stop(HeaterHandle_t *hheater);
void Heater_SetPower(HeaterHandle_t *hheater, uint16_t power);  // 0-999
uint16_t Heater_GetPower(HeaterHandle_t *hheater);
void Heater_Off(HeaterHandle_t *hheater);

#endif /* CORE_INC_HEATER_CONTROL_H_ */
