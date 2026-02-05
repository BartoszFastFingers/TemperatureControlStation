/**
 * @file heater_control.h
 * @brief Sterowanie grzałką za pomocą PWM na mikrokontrolerach STM32.
 *
 * Zawiera deklaracje funkcji i struktur dla inicjalizacji, uruchamiania,
 * zatrzymywania i ustawiania mocy grzałki.
 *
 * @author 00ary
 * @date 31-Jan-2026
 */

#ifndef CORE_INC_HEATER_CONTROL_H_
#define CORE_INC_HEATER_CONTROL_H_

#include <stdint.h>
#include "tim.h"

/**
 * @brief Struktura reprezentująca grzałkę sterowaną PWM.
 */
typedef struct
{
    TIM_HandleTypeDef *htim;   /**< Wskaźnik na strukturę timera STM32 używanego do PWM */
    uint32_t channel;           /**< Kanał timera dla PWM */
    uint16_t current_power;     /**< Aktualna moc grzałki (0-999) */
} HeaterHandle_t;

/**
 * @brief Inicjalizuje strukturę grzałki.
 * @param hheater Wskaźnik na strukturę HeaterHandle_t
 * @param htim Wskaźnik na strukturę timera STM32
 * @param channel Kanał timera dla PWM
 */
void Heater_Init(HeaterHandle_t *hheater,
                 TIM_HandleTypeDef *htim,
                 uint32_t channel);

/**
 * @brief Uruchamia PWM grzałki.
 * @param hheater Wskaźnik na strukturę HeaterHandle_t
 */
void Heater_Start(HeaterHandle_t *hheater);

/**
 * @brief Zatrzymuje PWM grzałki i ustawia moc na 0.
 * @param hheater Wskaźnik na strukturę HeaterHandle_t
 */
void Heater_Stop(HeaterHandle_t *hheater);

/**
 * @brief Ustawia moc grzałki.
 * @param hheater Wskaźnik na strukturę HeaterHandle_t
 * @param power Moc PWM w zakresie 0-999
 */
void Heater_SetPower(HeaterHandle_t *hheater, uint16_t power);

/**
 * @brief Pobiera aktualną moc grzałki.
 * @param hheater Wskaźnik na strukturę HeaterHandle_t
 * @return Aktualna moc PWM
 */
uint16_t Heater_GetPower(HeaterHandle_t *hheater);

/**
 * @brief Wyłącza grzałkę (ustawia moc na 0).
 * @param hheater Wskaźnik na strukturę HeaterHandle_t
 */
void Heater_Off(HeaterHandle_t *hheater);

#endif /* CORE_INC_HEATER_CONTROL_H_ */
