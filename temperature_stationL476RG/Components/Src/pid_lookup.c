/**
 * @file pid_lookup.c
 * @brief Implementacja interpolacji PID dla CMSIS-DSP 1.16.2
 *
 * CMSIS-DSP PID używa formuły:
 *   y[n] = y[n-1] + A0*x[n] + A1*x[n-1] + A2*x[n-2]
 * gdzie:
 *   A0 = Kp + Ki + Kd
 *   A1 = -Kp - 2*Kd
 *   A2 = Kd
 */

#include "pid_lookup.h"
#include <math.h>

// ============ Punkty pomiarowe (PWM 0-999) ============
static const uint16_t op_heater[NUM_OPERATING_POINTS] = {999, 499, 699};

static const uint16_t op_fan[NUM_OPERATING_POINTS] = {0, 499, 299};

// Parametry PID (forma ciągła)
static const float pid_Kp[NUM_OPERATING_POINTS] = {0.5f, 0.5f, 0.5f};
static const float pid_Ki[NUM_OPERATING_POINTS] = {0.05f, 0.05f, 0.05f};
static const float pid_Kd[NUM_OPERATING_POINTS] = {0.0f, 0.0f, 0.0f};


// ============ Interpolacja IDW ============
static void interpolate_params(uint16_t heater_pwm, uint16_t fan_pwm,
                               float *Kp_out, float *Ki_out, float *Kd_out)
{
    float weights[NUM_OPERATING_POINTS];
    float total_weight = 0.0f;
    const float epsilon = 0.5f;  // Tolerancja dla PWM

    for (int i = 0; i < NUM_OPERATING_POINTS; i++) {
        float dh = (float)heater_pwm - (float)op_heater[i];
        float df = (float)fan_pwm - (float)op_fan[i];
        float dist = sqrtf(dh*dh + df*df);

        if (dist < epsilon) {
            *Kp_out = pid_Kp[i];
            *Ki_out = pid_Ki[i];
            *Kd_out = pid_Kd[i];
            return;
        }

        weights[i] = 1.0f / (dist * dist);  // IDW power = 2
        total_weight += weights[i];
    }

    *Kp_out = 0.0f;
    *Ki_out = 0.0f;
    *Kd_out = 0.0f;

    for (int i = 0; i < NUM_OPERATING_POINTS; i++) {
        float w = weights[i] / total_weight;
        *Kp_out += w * pid_Kp[i];
        *Ki_out += w * pid_Ki[i];
        *Kd_out += w * pid_Kd[i];
    }
}

// ============ Aktualizacja instancji CMSIS-DSP ============
static void update_cmsis_pid(arm_pid_instance_f32 *pid, 
                             float Kp, float Ki, float Kd,
                             uint8_t reset_state)
{
    // Przelicz na współczynniki dyskretne CMSIS-DSP
    // CMSIS używa: y[n] = y[n-1] + A0*e[n] + A1*e[n-1] + A2*e[n-2]
    // Dla dyskretyzacji Tustin/Backward Euler:
    //   Ki_discrete = Ki * Ts
    //   Kd_discrete = Kd / Ts
    
    float Ki_d = Ki * PID_SAMPLE_TIME_S;
    float Kd_d = Kd / PID_SAMPLE_TIME_S;
    
    pid->Kp = Kp;
    pid->Ki = Ki_d;
    pid->Kd = Kd_d;
    
    // Oblicz współczynniki A0, A1, A2
    pid->A0 = Kp + Ki_d + Kd_d;
    pid->A1 = -Kp - 2.0f * Kd_d;
    pid->A2 = Kd_d;
    
    // Reset stanu jeśli wymagany
    if (reset_state) {
        pid->state[0] = 0.0f;  // e[n-1]
        pid->state[1] = 0.0f;  // e[n-2]
        pid->state[2] = 0.0f;  // y[n-1] (output)
    }
}

// ============ API publiczne ============

void PID_Init(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid)
{
    float Kp, Ki, Kd;
    interpolate_params(heater_pwm, fan_pwm, &Kp, &Ki, &Kd);
    update_cmsis_pid(pid, Kp, Ki, Kd, 1);  // Reset state
}

void PID_Interpolate(uint16_t heater_pwm, uint16_t fan_pwm,
                     arm_pid_instance_f32 *pid, uint8_t reset_state)
{
    float Kp, Ki, Kd;
    interpolate_params(heater_pwm, fan_pwm, &Kp, &Ki, &Kd);
    update_cmsis_pid(pid, Kp, Ki, Kd, reset_state);
}

void PID_GetNearest(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid)
{
    float min_dist = 1e9f;
    int nearest = 0;

    for (int i = 0; i < NUM_OPERATING_POINTS; i++) {
        float dh = (float)heater_pwm - (float)op_heater[i];
        float df = (float)fan_pwm - (float)op_fan[i];
        float dist = dh*dh + df*df;

        if (dist < min_dist) {
            min_dist = dist;
            nearest = i;
        }
    }

    update_cmsis_pid(pid, pid_Kp[nearest], pid_Ki[nearest], pid_Kd[nearest], 1);
}
