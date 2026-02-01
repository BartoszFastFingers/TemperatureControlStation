/*
 * fan_control.c
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#include "fan_control.h"

void Fan_Init(FanHandle_t *hfan,
              TIM_HandleTypeDef *htim,
              uint32_t channel)
{
    hfan->htim = htim;
    hfan->channel = channel;
    hfan->current_speed = 0;
}

void Fan_Start(FanHandle_t *hfan)
{
    HAL_TIM_PWM_Start(hfan->htim, hfan->channel);
}

void Fan_Stop(FanHandle_t *hfan)
{
    HAL_TIM_PWM_Stop(hfan->htim, hfan->channel);
    hfan->current_speed = 0;
}


void Fan_SetSpeed(FanHandle_t *hfan, uint16_t speed)
{
    if (speed > 999)
        speed = 999;

    __HAL_TIM_SET_COMPARE(hfan->htim, hfan->channel, speed);
    hfan->current_speed = speed;
}

uint16_t Fan_GetSpeed(FanHandle_t *hfan)
{
    return hfan->current_speed;
}

void Fan_Off(FanHandle_t *hfan)
{
    Fan_SetSpeed(hfan, 0);
}
