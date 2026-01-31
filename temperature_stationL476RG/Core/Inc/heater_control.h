/*
 * heater_control.h
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#ifndef CORE_INC_HEATER_CONTROL_H_
#define CORE_INC_HEATER_CONTROL_H_

#include "tim.h"
#include <stdint.h>


typedef struct
{
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint16_t max_duty;
    uint8_t current_power;
} HeaterHandle_t;

void Heater_Init(HeaterHandle_t *hheater,
                 TIM_HandleTypeDef *htim,
                 uint32_t channel,
                 uint16_t max_duty);

void Heater_Start(HeaterHandle_t *hheater);
void Heater_Stop(HeaterHandle_t *hheater);
void Heater_SetPower(HeaterHandle_t *hheater, uint8_t power_percent);
void Heater_SetDuty(HeaterHandle_t *hheater, uint16_t duty_value);
uint8_t Heater_GetPower(HeaterHandle_t *hheater);
void Heater_Off(HeaterHandle_t *hheater);


#endif /* CORE_INC_HEATER_CONTROL_H_ */
