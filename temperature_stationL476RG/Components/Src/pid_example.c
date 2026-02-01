/**
 * @file pid_example.c
 * @brief Przykład użycia PID z interpolacją i CMSIS-DSP
 */

#include "pid_lookup.h"
#include "arm_math.h"

// Instancja PID
static arm_pid_instance_f32 pid_temp;

// Zmienne globalne
static uint16_t heater_pwm = 699;  // Aktualne PWM grzałki [0-999]
static uint16_t fan_pwm = 299;     // Aktualne PWM wentylatora [0-999]
static float temperature_setpoint = 35.0f;  // Zadana temperatura [°C]
static float output_min = 0.0f;    // Min wyjście PID
static float output_max = 999.0f;  // Max wyjście PID (PWM)

/**
 * @brief Inicjalizacja systemu
 */
void Temperature_Control_Init(void)
{
    // Inicjalizuj PID dla początkowego punktu pracy
    PID_Init(heater_pwm, fan_pwm, &pid_temp);
}

/**
 * @brief Główna pętla regulacji (wywoływana co Ts = 0.5s)
 * @param current_temp Aktualna temperatura [°C]
 * @return Wyjście regulatora [0-999 PWM]
 */
float Temperature_Control_Update(float current_temp)
{
    // 1. Aktualizuj parametry PID dla aktualnego punktu pracy
    //    (bez resetowania stanu - zachowujemy historię błędów)
    PID_Interpolate(heater_pwm, fan_pwm, &pid_temp, 0);
    
    // 2. Oblicz błąd regulacji
    float error = temperature_setpoint - current_temp;
    
    // 3. Wywołaj CMSIS-DSP PID
    float output = arm_pid_f32(&pid_temp, error);
    
    // 4. Saturacja wyjścia do zakresu PWM
    if (output > output_max) output = output_max;
    if (output < output_min) output = output_min;
    
    // 5. Aktualizuj PWM grzałki
    heater_pwm = (uint16_t)output;
    
    return output;
}

/**
 * @brief Zmiana setpointu
 */
void Temperature_Control_SetTarget(float target_temp)
{
    temperature_setpoint = target_temp;
}

/**
 * @brief Ustawienie PWM wentylatora (ręczne lub z innego regulatora)
 */
void Temperature_Control_SetFanPWM(uint16_t pwm)
{
    fan_pwm = (pwm > 999) ? 999 : pwm;
}
