 /*
 * radiator_control.c
 *
 *  Created on: 31 sty 2026
 *      Author: 00ary
 */

#include "radiato"

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
