/*
 * PID_controller.c
 *
 *  Created on: 1 __lut__ 2026
 *      Author: 00ary
 *
 *  Regulacja temperatury z podziałem grzanie/chłodzenie:
 *  - Gdy temp < setpoint → grzałka ON, fan OFF (lub minimum)
 *  - Gdy temp > setpoint → grzałka OFF, fan ON (proporcjonalnie)
 */

#include "PID_controller.h"

arm_pid_instance_f32 pid_heat;  // PID dla grzania
arm_pid_instance_f32 pid_cool;  // PID dla chłodzenia

static uint16_t heater_pwm = 0;
static uint16_t fan_pwm = 0;
static float temperature_setpoint = 35.0f;

// Limity PWM
static const uint16_t PWM_MIN = 0;


// Strefa martwa (deadband) - zapobiega ciągłemu przełączaniu
static const float DEADBAND = 1.0f;  // ±1.0°C - większa strefa

// Minimalna prędkość wentylatora (dla cyrkulacji powietrza)
static const uint16_t FAN_MIN = 0;

// Współczynnik wygładzania (0.0-1.0, mniejszy = wolniejsza reakcja)
static const float SMOOTHING = 0.3f;

void Temperature_Control_Init(void)
{
    // Inicjalizuj PID dla grzania (domyślny punkt pracy)
    PID_Init(699, 299, &pid_heat);

    // Inicjalizuj PID dla chłodzenia (punkt pracy: fan dominuje)
    // Używamy tych samych parametrów, ale możesz dostosować
    PID_Init(0, 999, &pid_cool);

    heater_pwm = 0;
    fan_pwm = FAN_MIN;
}

float Temperature_Control_Update(float current_temp)
{
    float error = temperature_setpoint - current_temp;
    float output;
    static float last_heater_output = 0.0f;
    static float last_fan_output = 0.0f;

    if (error > DEADBAND) {
        // ========== ZA ZIMNO - GRZANIE ==========
        PID_Interpolate(heater_pwm, fan_pwm, &pid_heat, 0);

        output = arm_pid_f32(&pid_heat, error);

        // Saturacja
        if (output > PWM_MAX) output = PWM_MAX;
        if (output < PWM_MIN) output = PWM_MIN;

        // Wygładzanie - powolna zmiana
        last_heater_output = last_heater_output + SMOOTHING * (output - last_heater_output);
        last_fan_output = last_fan_output + SMOOTHING * (FAN_MIN - last_fan_output);

        heater_pwm = (uint16_t)last_heater_output;
        fan_pwm = (uint16_t)last_fan_output;

        // Reset PID chłodzenia
        pid_cool.state[0] = 0.0f;
        pid_cool.state[1] = 0.0f;
        pid_cool.state[2] = 0.0f;

    } else if (error < -DEADBAND) {
        // ========== ZA CIEPŁO - CHŁODZENIE ==========
        PID_Interpolate(heater_pwm, fan_pwm, &pid_cool, 0);

        float cool_error = -error;
        output = arm_pid_f32(&pid_cool, cool_error);

        // Saturacja
        if (output > PWM_MAX) output = PWM_MAX;
        if (output < PWM_MIN) output = PWM_MIN;

        // Wygładzanie
        last_heater_output = last_heater_output + SMOOTHING * (PWM_MIN - last_heater_output);
        last_fan_output = last_fan_output + SMOOTHING * (output - last_fan_output);

        heater_pwm = (uint16_t)last_heater_output;
        fan_pwm = (uint16_t)last_fan_output;

        // Reset PID grzania
        pid_heat.state[0] = 0.0f;
        pid_heat.state[1] = 0.0f;
        pid_heat.state[2] = 0.0f;

    } else {
        // ========== W STREFIE MARTWEJ ==========
        // Powoli redukuj oba wyjścia
        last_heater_output *= 0.95f;
        last_fan_output *= 0.95f;

        heater_pwm = (uint16_t)last_heater_output;
        fan_pwm = (uint16_t)last_fan_output;
    }

    return (float)heater_pwm;
}

void Temperature_Control_SetTarget(float target_temp)
{
    // Przy zmianie setpointu resetuj stany PID
    if (target_temp != temperature_setpoint) {
        pid_heat.state[0] = 0.0f;
        pid_heat.state[1] = 0.0f;
        pid_heat.state[2] = 0.0f;

        pid_cool.state[0] = 0.0f;
        pid_cool.state[1] = 0.0f;
        pid_cool.state[2] = 0.0f;
    }
    temperature_setpoint = target_temp + 1.0f;
}

void Temperature_Control_SetFanPWM(uint16_t pwm)
{
    // Ta funkcja teraz może służyć do ręcznego override
    // Normalnie fan jest sterowany automatycznie
    fan_pwm = (pwm > PWM_MAX) ? PWM_MAX : pwm;
}

uint16_t Temperature_Control_GetHeaterPWM(void)
{
    return heater_pwm;
}

uint16_t Temperature_Control_GetFanPWM(void)
{
    return fan_pwm;
}

float Temperature_Control_GetSetpoint(void)
{
    return temperature_setpoint;
}
