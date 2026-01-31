/*
 * temperature_sensor.c
 *
 *  Created on: 30 sty 2026
 *      Author: 00ary
 */


#include "temperature_sensor.h"


void TempSensor_Init(TempSensorHandle_t *htemp,
                     ADC_HandleTypeDef *hadc,
                     uint8_t sensor_count,
					 uint16_t* adc_buffer,
                     float vref)
{
    htemp->hadc = hadc;
    htemp->sensor_count = sensor_count;
    htemp->adc_buffer = adc_buffer;
    htemp->vref = vref;

    arm_fir_init_f32(&htemp->filter.fir, SMA1_NUM_TAPS, SMA1_COEFFS, htemp->filter.state, SMA1_BLOCK_SIZE);
}

void TempSensor_Start(TempSensorHandle_t *htemp)
{
    HAL_ADC_Start_DMA(
        htemp->hadc,
        (uint32_t *)htemp->adc_buffer,
        htemp->sensor_count
    );
}

float TempSensor_GetVoltage(TempSensorHandle_t *htemp, uint8_t index)
{
    if (index >= htemp->sensor_count)
        return 0.0f;

    return (htemp->adc_buffer[index] * htemp->vref) / 4095.0f;
}

float TempSensor_GetTemperature(TempSensorHandle_t *htemp, uint8_t index)
{
    float voltage = TempSensor_GetVoltage(htemp, index);
    return voltage * 100.0f;
}

float TempSensor_GetAverageTemperature(TempSensorHandle_t *htemp)
{
    if (htemp->sensor_count == 0)
        return 0.0f;

    float sum = 0.0f;

    for (uint8_t i = 0; i < htemp->sensor_count; i++)
    {
        sum += TempSensor_GetTemperature(htemp, i);
    }

    float raw_average = sum / htemp->sensor_count;
    float filtered_average = 0.0f;

    arm_fir_f32(&htemp->filter.fir, &raw_average, &filtered_average, 1);

    return filtered_average;
}
