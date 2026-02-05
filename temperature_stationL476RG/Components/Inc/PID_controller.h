/**
 * @file PID_controller.h
 * @brief Regulator PID temperatury
 * @author 00ary
 * @date 2026-02-01
 */

#ifndef COMPONENTS_INC_PID_CONTROLLER_H_
#define COMPONENTS_INC_PID_CONTROLLER_H_

#include "pid_lookup.h"

extern arm_pid_instance_f32 pid_heat;
extern arm_pid_instance_f32 pid_cool;

/**
 * @brief Inicjalizacja regulatora temperatury
 */
void Temperature_Control_Init(void);

/**
 * @brief Aktualizacja regulatora (wywoływać co 500ms)
 * @param current_temp Aktualna temperatura [°C]
 * @return PWM grzałki [0-999]
 */
float Temperature_Control_Update(float current_temp);

/**
 * @brief Ustawienie temperatury zadanej
 * @param target_temp Temperatura zadana [°C]
 */
void Temperature_Control_SetTarget(float target_temp);

/**
 * @brief Ręczne ustawienie PWM wentylatora (override)
 * @param pwm Wartość PWM [0-999]
 */
void Temperature_Control_SetFanPWM(uint16_t pwm);

/**
 * @brief Pobierz aktualne PWM grzałki
 */
uint16_t Temperature_Control_GetHeaterPWM(void);

/**
 * @brief Pobierz aktualne PWM wentylatora
 */
uint16_t Temperature_Control_GetFanPWM(void);

/**
 * @brief Pobierz aktualny setpoint
 */
float Temperature_Control_GetSetpoint(void);

#endif /* COMPONENTS_INC_PID_CONTROLLER_H_ */
