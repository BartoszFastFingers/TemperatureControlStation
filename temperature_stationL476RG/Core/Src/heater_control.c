/*
 * heater_control.c
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#include "heater_control.h"

void Heater_Init(HeaterHandle_t *hheater,
                 TIM_HandleTypeDef *htim,
                 uint32_t channel)
{
    hheater->htim = htim;
    hheater->channel = channel;
    hheater->current_power = 0;
}

void Heater_Start(HeaterHandle_t *hheater)
{
    HAL_TIM_PWM_Start(hheater->htim, hheater->channel);
}

void Heater_Stop(HeaterHandle_t *hheater)
{
    HAL_TIM_PWM_Stop(hheater->htim, hheater->channel);
    hheater->current_power = 0;
}

void Heater_SetPower(HeaterHandle_t *hheater, uint16_t power)
{
    if (power > 999)
        power = 999;

    __HAL_TIM_SET_COMPARE(hheater->htim, hheater->channel, power);

    hheater->current_power = power;
}

uint16_t Heater_GetPower(HeaterHandle_t *hheater)
{
    return hheater->current_power;
}

void Heater_Off(HeaterHandle_t *hheater)
{
    Heater_SetPower(hheater, 0);
}
