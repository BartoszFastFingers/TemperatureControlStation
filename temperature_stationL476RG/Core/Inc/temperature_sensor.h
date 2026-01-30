/*
 * temperature_sensor.h
 *
 *  Created on: 30 sty 2026
 *      Author: 00ary
 */

#ifndef CORE_INC_TEMPERATURE_SENSOR_H_
#define CORE_INC_TEMPERATURE_SENSOR_H_





#include "adc.h"
#include <stdint.h>

#define MAX_TEMP_SENSORS 2


typedef struct
{
    ADC_HandleTypeDef *hadc;
    uint8_t sensor_count;
    uint16_t *adc_buffer;
    float vref;
} TempSensorHandle_t;

void TempSensor_Init(TempSensorHandle_t *htemp,
                     ADC_HandleTypeDef *hadc,
                     uint8_t sensor_count,
					 uint16_t* adc_buffer,
                     float vref);

void TempSensor_Start(TempSensorHandle_t *htemp);

float TempSensor_GetVoltage(TempSensorHandle_t *htemp, uint8_t index);
float TempSensor_GetTemperature(TempSensorHandle_t *htemp, uint8_t index);
float TempSensor_GetAverageTemperature(TempSensorHandle_t *htemp);

#endif /* CORE_INC_TEMPERATURE_SENSOR_H_ */
