/*
 * temperature_sensor.h
 *
 *  Created on: 30 sty 2026
 *      Author: 00ary
 */

#ifndef CORE_INC_TEMPERATURE_SENSOR_H_
#define CORE_INC_TEMPERATURE_SENSOR_H_


#include <stdint.h>
#include "adc.h"
#include "arm_math.h"
#include "SMA1_fir.h"

#define MAX_TEMP_SENSORS 2


typedef struct
{
	arm_fir_instance_f32 fir;
	float32_t state[SMA1_NUM_TAPS + SMA1_BLOCK_SIZE - 1];
}TempSensorHandle_filter_t;

typedef struct
{
    ADC_HandleTypeDef *hadc;
    uint8_t sensor_count;
    uint16_t *adc_buffer;
    float vref;

    TempSensorHandle_filter_t filter;
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
