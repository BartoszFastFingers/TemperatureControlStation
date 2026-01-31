/*
 * heater_control.c
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#include "heater_control.h"

void Heater_Init(HeaterHandle_t *hheater,
                 TIM_HandleTypeDef *htim,
                 uint32_t channel,
                 uint16_t max_duty)
{
    hheater->htim = htim;
    hheater->channel = channel;
    hheater->max_duty = max_duty;
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

void Heater_SetPower(HeaterHandle_t *hheater, uint8_t power_percent)
{
    if (power_percent > 100)
        power_percent = 100;
    uint16_t ccr_value = (power_percent * hheater->max_duty) / 100;

    __HAL_TIM_SET_COMPARE(hheater->htim, hheater->channel, ccr_value);

    hheater->current_power = power_percent;
}

void Heater_SetDuty(HeaterHandle_t *hheater, uint16_t duty_value)
{
    if (duty_value > hheater->max_duty)
        duty_value = hheater->max_duty;

    __HAL_TIM_SET_COMPARE(hheater->htim, hheater->channel, duty_value);

    hheater->current_power = (duty_value * 100) / hheater->max_duty;
}

uint8_t Heater_GetPower(HeaterHandle_t *hheater)
{
    return hheater->current_power;
}

void Heater_Off(HeaterHandle_t *hheater)
{
    Heater_SetPower(hheater, 0);
}
