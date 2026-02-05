/**
 * @file temperature_sensor.h
 * @brief Obsługa czujników temperatury z filtrem FIR (SMA) dla mikrokontrolerów STM32.
 *
 * Moduł umożliwia:
 * - inicjalizację czujników ADC,
 * - odczyt napięcia i temperatury,
 * - obliczanie średniej temperatury z filtracją cyfrową (SMA/FIR).
 *
 * @author 00ary
 * @date 30-Jan-2026
 */

#ifndef CORE_INC_TEMPERATURE_SENSOR_H_
#define CORE_INC_TEMPERATURE_SENSOR_H_

#include <stdint.h>
#include "adc.h"
#include "arm_math.h"
#include "SMA1_fir.h"

/**
 * @brief Maksymalna liczba czujników temperatury obsługiwanych w systemie
 */
#define MAX_TEMP_SENSORS 2

/**
 * @brief Struktura przechowująca dane filtra FIR dla pojedynczego czujnika.
 */
typedef struct
{
    arm_fir_instance_f32 fir;                      /**< Instancja filtra CMSIS-DSP */
    float32_t state[SMA1_NUM_TAPS + SMA1_BLOCK_SIZE - 1]; /**< Bufor stanu filtra */
} TempSensorHandle_filter_t;

/**
 * @brief Struktura reprezentująca jeden lub więcej czujników temperatury.
 */
typedef struct
{
    ADC_HandleTypeDef *hadc;             /**< Wskaźnik na strukturę ADC STM32 */
    uint8_t sensor_count;                /**< Liczba czujników w systemie */
    uint16_t *adc_buffer;                /**< Bufor ADC do przechowywania próbek */
    float vref;                           /**< Napięcie odniesienia ADC */

    TempSensorHandle_filter_t filter;    /**< Struktura filtra FIR dla uśredniania temperatury */
} TempSensorHandle_t;

/**
 * @brief Inicjalizuje strukturę czujnika temperatury i filtr SMA.
 * @param htemp Wskaźnik na strukturę TempSensorHandle_t
 * @param hadc Wskaźnik na strukturę ADC STM32
 * @param sensor_count Liczba czujników do obsługi
 * @param adc_buffer Bufor do odczytu ADC
 * @param vref Napięcie odniesienia ADC
 */
void TempSensor_Init(TempSensorHandle_t *htemp,
                     ADC_HandleTypeDef *hadc,
                     uint8_t sensor_count,
                     uint16_t* adc_buffer,
                     float vref);

/**
 * @brief Uruchamia odczyt ADC z czujników temperatury z użyciem DMA.
 * @param htemp Wskaźnik na strukturę TempSensorHandle_t
 */
void TempSensor_Start(TempSensorHandle_t *htemp);

/**
 * @brief Pobiera napięcie z wybranego czujnika.
 * @param htemp Wskaźnik na strukturę TempSensorHandle_t
 * @param index Indeks czujnika (0..sensor_count-1)
 * @return Napięcie w woltach
 */
float TempSensor_GetVoltage(TempSensorHandle_t *htemp, uint8_t index);

/**
 * @brief Pobiera temperaturę z wybranego czujnika.
 * @param htemp Wskaźnik na strukturę TempSensorHandle_t
 * @param index Indeks czujnika (0..sensor_count-1)
 * @return Temperatura w °C
 */
float TempSensor_GetTemperature(TempSensorHandle_t *htemp, uint8_t index);

/**
 * @brief Oblicza średnią temperaturę ze wszystkich czujników z filtracją SMA.
 * @param htemp Wskaźnik na strukturę TempSensorHandle_t
 * @return Przefiltrowana średnia temperatura w °C
 */
float TempSensor_GetAverageTemperature(TempSensorHandle_t *htemp);

#endif /* CORE_INC_TEMPERATURE_SENSOR_H_ */
