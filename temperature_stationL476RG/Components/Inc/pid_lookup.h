/**
 * @file pid_lookup.h
 * @brief Interpolacja parametrów PID z CMSIS-DSP 1.16.2
 * @note Wygenerowano automatycznie przez MATLAB
 * @date 2026-02-01 03:03
 */

#ifndef PID_LOOKUP_H
#define PID_LOOKUP_H

#include "arm_math.h"

#define NUM_OPERATING_POINTS 3
#define IDW_POWER            2.0f
#define PID_SAMPLE_TIME_S    1.0f  // Dostosuj do swojego Ts!
#define PWM_MAX              999   // Maksymalna wartość PWM

/**
 * @brief Interpoluje parametry PID i aktualizuje instancję CMSIS-DSP
 * @param heater_pwm Wartość PWM grzałki [0-999]
 * @param fan_pwm Wartość PWM wentylatora [0-999]
 * @param pid Wskaźnik na instancję arm_pid_instance_f32
 * @param reset_state Czy zresetować stan całkowania (1=tak, 0=nie)
 */
void PID_Interpolate(uint16_t heater_pwm, uint16_t fan_pwm, 
                     arm_pid_instance_f32 *pid, uint8_t reset_state);

/**
 * @brief Inicjalizuje PID dla danego punktu pracy
 */
void PID_Init(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid);

/**
 * @brief Zwraca parametry dla najbliższego punktu (bez interpolacji)
 */
void PID_GetNearest(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid);

#endif // PID_LOOKUP_H
