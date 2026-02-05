/**
 * @file fan_control.h
 * @brief Sterowanie wentylatorem za pomocą TIM PWM na mikrokontrolerach STM32.
 *
 * Zawiera deklaracje funkcji i struktur dla inicjalizacji, uruchamiania,
 * zatrzymywania i ustawiania prędkości wentylatora.
 *
 * @author 00ary
 * @date 31-Jan-2026
 */

#ifndef CORE_INC_FAN_CONTROL_H_
#define CORE_INC_FAN_CONTROL_H_

#include <stdint.h>
#include "tim.h"

/**
 * @brief Struktura reprezentująca wentylator sterowany PWM.
 */
typedef struct
{
    TIM_HandleTypeDef *htim; /**< Wskaźnik na strukturę timera STM32 używanego do PWM */
    uint32_t channel;         /**< Kanał timera dla PWM */
    uint16_t current_speed;   /**< Aktualna prędkość wentylatora (0-999) */
} FanHandle_t;

/**
 * @brief Inicjalizuje strukturę wentylatora.
 * @param hfan Wskaźnik na strukturę FanHandle_t
 * @param htim Wskaźnik na strukturę timera STM32
 * @param channel Kanał timera dla PWM
 */
void Fan_Init(FanHandle_t *hfan,
              TIM_HandleTypeDef *htim,
              uint32_t channel);

/**
 * @brief Uruchamia PWM wentylatora.
 * @param hfan Wskaźnik na strukturę FanHandle_t
 */
void Fan_Start(FanHandle_t *hfan);

/**
 * @brief Zatrzymuje PWM wentylatora i ustawia prędkość na 0.
 * @param hfan Wskaźnik na strukturę FanHandle_t
 */
void Fan_Stop(FanHandle_t *hfan);

/**
 * @brief Ustawia prędkość wentylatora.
 * @param hfan Wskaźnik na strukturę FanHandle_t
 * @param speed Prędkość PWM w zakresie 0-999
 */
void Fan_SetSpeed(FanHandle_t *hfan, uint16_t speed);

/**
 * @brief Pobiera aktualną prędkość wentylatora.
 * @param hfan Wskaźnik na strukturę FanHandle_t
 * @return Aktualna prędkość PWM
 */
uint16_t Fan_GetSpeed(FanHandle_t *hfan);

/**
 * @brief Wyłącza wentylator (ustawia prędkość na 0).
 * @param hfan Wskaźnik na strukturę FanHandle_t
 */
void Fan_Off(FanHandle_t *hfan);

#endif /* CORE_INC_FAN_CONTROL_H_ */
