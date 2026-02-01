%% INTERPOLATE_PID - Interpolacja parametrów PID w funkcji punktu pracy

clear; clc; close all;

%% Wczytaj parametry PID
if ~isfile('pid_params.mat')
    error('Najpierw uruchom tune_pid.m!');
end

load('pid_params.mat', 'pid_params');

%% Wyodrębnij dane do interpolacji
heater = [pid_params.heater]';
fan = [pid_params.fan]';
Kp = [pid_params.Kp]';
Ki = [pid_params.Ki]';
Kd = [pid_params.Kd]';

fprintf('Punkty pomiarowe:\n');
for i = 1:length(heater)
    fprintf('  [H=%3d%%, F=%3d%%] -> Kp=%.4f, Ki=%.4f, Kd=%.4f\n', ...
        heater(i), fan(i), Kp(i), Ki(i), Kd(i));
end

%% Sprawdź wartości - ujemne Ki/Kd dla chłodzenia są problematyczne
fprintf('\n⚠ UWAGA: Pomiar 2 (sam wentylator) ma ujemne parametry!\n');
fprintf('  To normalne dla procesu chłodzenia (K < 0).\n');
fprintf('  Ale w interpolacji może to dawać dziwne wyniki.\n\n');

%% Metoda IDW (Inverse Distance Weighting)
% Działa dla dowolnej liczby punktów, nie wymaga triangulacji

fprintf('========== TWORZENIE INTERPOLATORA IDW ==========\n');

% Parametr potęgi IDW (2 = standardowe, wyższe = bardziej lokalne)
idw_power = 2;

%% Funkcja interpolacji IDW (inline)
idw_interpolate = @(h, f, values) idw_calc(h, f, heater, fan, values, idw_power);

%% Wizualizacja powierzchni interpolacji
[H_grid, F_grid] = meshgrid(0:50:999, 0:50:999);

% Interpolacja na siatce
Kp_grid = zeros(size(H_grid));
Ki_grid = zeros(size(H_grid));
Kd_grid = zeros(size(H_grid));

fprintf('Interpolacja na siatce 20x20 (PWM 0-999)...\n');

for i = 1:size(H_grid, 1)
    for j = 1:size(H_grid, 2)
        h = H_grid(i, j);
        f = F_grid(i, j);
        Kp_grid(i, j) = idw_interpolate(h, f, Kp);
        Ki_grid(i, j) = idw_interpolate(h, f, Ki);
        Kd_grid(i, j) = idw_interpolate(h, f, Kd);
    end
end

fprintf('Gotowe!\n\n');

%% Wykresy 3D
figure('Name', 'Mapy parametrów PID', 'Position', [50, 50, 1400, 900]);

% Kp
subplot(2,3,1);
surf(H_grid, F_grid, Kp_grid, 'FaceAlpha', 0.8, 'EdgeAlpha', 0.3);
hold on;
scatter3(heater, fan, Kp, 100, 'r', 'filled');
xlabel('Heater PWM');
ylabel('Fan PWM');
zlabel('Kp');
title('Wzmocnienie proporcjonalne Kp');
colorbar;
view(45, 30);

% Ki
subplot(2,3,2);
surf(H_grid, F_grid, Ki_grid, 'FaceAlpha', 0.8, 'EdgeAlpha', 0.3);
hold on;
scatter3(heater, fan, Ki, 100, 'r', 'filled');
xlabel('Heater PWM');
ylabel('Fan PWM');
zlabel('Ki');
title('Wzmocnienie całkujące Ki');
colorbar;
view(45, 30);

% Kd
subplot(2,3,3);
surf(H_grid, F_grid, Kd_grid, 'FaceAlpha', 0.8, 'EdgeAlpha', 0.3);
hold on;
scatter3(heater, fan, Kd, 100, 'r', 'filled');
xlabel('Heater PWM');
ylabel('Fan PWM');
zlabel('Kd');
title('Wzmocnienie różniczkujące Kd');
colorbar;
view(45, 30);

% Kontury 2D
subplot(2,3,4);
contourf(H_grid, F_grid, Kp_grid, 20);
hold on;
scatter(heater, fan, 100, 'r', 'filled');
xlabel('Heater PWM');
ylabel('Fan PWM');
title('Kontur Kp');
colorbar;

subplot(2,3,5);
contourf(H_grid, F_grid, Ki_grid, 20);
hold on;
scatter(heater, fan, 100, 'r', 'filled');
xlabel('Heater PWM');
ylabel('Fan PWM');
title('Kontur Ki');
colorbar;

subplot(2,3,6);
contourf(H_grid, F_grid, Kd_grid, 20);
hold on;
scatter(heater, fan, 100, 'r', 'filled');
xlabel('Heater PWM');
ylabel('Fan PWM');
title('Kontur Kd');
colorbar;

sgtitle('Interpolacja parametrów PID (PWM 0-999, metoda IDW)');

%% Test interpolacji - przykładowe punkty
fprintf('========== TEST INTERPOLACJI ==========\n');

test_points = [
    800, 200;   % Punkt blisko 699/299
    600, 400;   % Punkt między pomiarami
    300, 700;   % Więcej chłodzenia
    900, 100;   % Prawie samo grzanie
    999, 0;     % Dokładnie punkt pomiarowy
    699, 299;   % Dokładnie punkt pomiarowy
];

fprintf('\n%-20s | %10s | %10s | %10s\n', 'Punkt [H,F]', 'Kp', 'Ki', 'Kd');
fprintf('%s\n', repmat('-', 1, 60));

for i = 1:size(test_points, 1)
    h = test_points(i, 1);
    f = test_points(i, 2);
    
    kp_interp = idw_interpolate(h, f, Kp);
    ki_interp = idw_interpolate(h, f, Ki);
    kd_interp = idw_interpolate(h, f, Kd);
    
    fprintf('[%4d, %4d]         | %10.4f | %10.4f | %10.4f\n', ...
        h, f, kp_interp, ki_interp, kd_interp);
end

%% Zapisz dane do interpolacji
interp_data.heater = heater;
interp_data.fan = fan;
interp_data.Kp = Kp;
interp_data.Ki = Ki;
interp_data.Kd = Kd;
interp_data.idw_power = idw_power;

save('pid_interpolators.mat', 'interp_data');
fprintf('\nDane interpolacji zapisane do: pid_interpolators.mat\n');

%% Generuj kod C dla STM32
generate_c_code(heater, fan, Kp, Ki, Kd, idw_power);

%% ==================== FUNKCJE POMOCNICZE ====================

function val = idw_calc(h, f, heater_pts, fan_pts, values, power)
    % IDW_CALC - Inverse Distance Weighting interpolation
    %
    % h, f       - punkt do interpolacji
    % heater_pts - współrzędne x punktów pomiarowych  
    % fan_pts    - współrzędne y punktów pomiarowych
    % values     - wartości w punktach pomiarowych
    % power      - wykładnik (zazwyczaj 2)
    
    n = length(heater_pts);
    epsilon = 0.001;  % Unikamy dzielenia przez zero
    
    weights = zeros(n, 1);
    total_weight = 0;
    
    for i = 1:n
        dist = sqrt((h - heater_pts(i))^2 + (f - fan_pts(i))^2);
        
        % Jeśli jesteśmy dokładnie w punkcie pomiarowym
        if dist < epsilon
            val = values(i);
            return;
        end
        
        weights(i) = 1 / (dist ^ power);
        total_weight = total_weight + weights(i);
    end
    
    % Oblicz ważoną średnią
    val = sum(weights .* values) / total_weight;
end

function generate_c_code(heater, fan, Kp, Ki, Kd, idw_power)
    % GENERATE_C_CODE - Generuje kod C zgodny z CMSIS-DSP 1.16.2
    
    fprintf('\n========== KOD C DLA STM32 (CMSIS-DSP 1.16.2) ==========\n\n');
    
    n = length(heater);
    
    %% ============ pid_lookup.h ============
    fid = fopen('pid_lookup.h', 'w');
    
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @file pid_lookup.h\n');
    fprintf(fid, ' * @brief Interpolacja parametrów PID z CMSIS-DSP 1.16.2\n');
    fprintf(fid, ' * @note Wygenerowano automatycznie przez MATLAB\n');
    fprintf(fid, ' * @date %s\n', datestr(now, 'yyyy-mm-dd HH:MM'));
    fprintf(fid, ' */\n\n');
    fprintf(fid, '#ifndef PID_LOOKUP_H\n');
    fprintf(fid, '#define PID_LOOKUP_H\n\n');
    fprintf(fid, '#include "arm_math.h"\n\n');
    fprintf(fid, '#define NUM_OPERATING_POINTS %d\n', n);
    fprintf(fid, '#define IDW_POWER            2.0f\n');
    fprintf(fid, '#define PID_SAMPLE_TIME_S    0.5f  // Dostosuj do swojego Ts!\n');
    fprintf(fid, '#define PWM_MAX              999   // Maksymalna wartość PWM\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Interpoluje parametry PID i aktualizuje instancję CMSIS-DSP\n');
    fprintf(fid, ' * @param heater_pwm Wartość PWM grzałki [0-999]\n');
    fprintf(fid, ' * @param fan_pwm Wartość PWM wentylatora [0-999]\n');
    fprintf(fid, ' * @param pid Wskaźnik na instancję arm_pid_instance_f32\n');
    fprintf(fid, ' * @param reset_state Czy zresetować stan całkowania (1=tak, 0=nie)\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'void PID_Interpolate(uint16_t heater_pwm, uint16_t fan_pwm, \n');
    fprintf(fid, '                     arm_pid_instance_f32 *pid, uint8_t reset_state);\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Inicjalizuje PID dla danego punktu pracy\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'void PID_Init(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid);\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Zwraca parametry dla najbliższego punktu (bez interpolacji)\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'void PID_GetNearest(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid);\n\n');
    fprintf(fid, '#endif // PID_LOOKUP_H\n');
    fclose(fid);
    
    %% ============ pid_lookup.c ============
    fid = fopen('pid_lookup.c', 'w');
    
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @file pid_lookup.c\n');
    fprintf(fid, ' * @brief Implementacja interpolacji PID dla CMSIS-DSP 1.16.2\n');
    fprintf(fid, ' *\n');
    fprintf(fid, ' * CMSIS-DSP PID używa formuły:\n');
    fprintf(fid, ' *   y[n] = y[n-1] + A0*x[n] + A1*x[n-1] + A2*x[n-2]\n');
    fprintf(fid, ' * gdzie:\n');
    fprintf(fid, ' *   A0 = Kp + Ki + Kd\n');
    fprintf(fid, ' *   A1 = -Kp - 2*Kd\n');
    fprintf(fid, ' *   A2 = Kd\n');
    fprintf(fid, ' */\n\n');
    fprintf(fid, '#include "pid_lookup.h"\n');
    fprintf(fid, '#include <math.h>\n\n');
    
    % Dane punktów pomiarowych
    fprintf(fid, '// ============ Punkty pomiarowe (PWM 0-999) ============\n');
    fprintf(fid, 'static const uint16_t op_heater[NUM_OPERATING_POINTS] = {');
    fprintf(fid, '%d', heater(1));
    for i = 2:n
        fprintf(fid, ', %d', heater(i));
    end
    fprintf(fid, '};\n\n');
    
    fprintf(fid, 'static const uint16_t op_fan[NUM_OPERATING_POINTS] = {');
    fprintf(fid, '%d', fan(1));
    for i = 2:n
        fprintf(fid, ', %d', fan(i));
    end
    fprintf(fid, '};\n\n');
    
    % Parametry PID (Kp, Ki, Kd w formie ciągłej - przeliczane na dyskretne)
    fprintf(fid, '// Parametry PID (forma ciągła)\n');
    fprintf(fid, 'static const float pid_Kp[NUM_OPERATING_POINTS] = {');
    fprintf(fid, '%.6ff', Kp(1));
    for i = 2:n
        fprintf(fid, ', %.6ff', Kp(i));
    end
    fprintf(fid, '};\n\n');
    
    fprintf(fid, 'static const float pid_Ki[NUM_OPERATING_POINTS] = {');
    fprintf(fid, '%.6ff', Ki(1));
    for i = 2:n
        fprintf(fid, ', %.6ff', Ki(i));
    end
    fprintf(fid, '};\n\n');
    
    fprintf(fid, 'static const float pid_Kd[NUM_OPERATING_POINTS] = {');
    fprintf(fid, '%.6ff', Kd(1));
    for i = 2:n
        fprintf(fid, ', %.6ff', Kd(i));
    end
    fprintf(fid, '};\n\n');
    
    % Funkcja pomocnicza IDW
    fprintf(fid, '// ============ Interpolacja IDW ============\n');
    fprintf(fid, 'static void interpolate_params(uint16_t heater_pwm, uint16_t fan_pwm,\n');
    fprintf(fid, '                               float *Kp_out, float *Ki_out, float *Kd_out)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    float weights[NUM_OPERATING_POINTS];\n');
    fprintf(fid, '    float total_weight = 0.0f;\n');
    fprintf(fid, '    const float epsilon = 0.5f;  // Tolerancja dla PWM\n\n');
    fprintf(fid, '    for (int i = 0; i < NUM_OPERATING_POINTS; i++) {\n');
    fprintf(fid, '        float dh = (float)heater_pwm - (float)op_heater[i];\n');
    fprintf(fid, '        float df = (float)fan_pwm - (float)op_fan[i];\n');
    fprintf(fid, '        float dist = sqrtf(dh*dh + df*df);\n\n');
    fprintf(fid, '        if (dist < epsilon) {\n');
    fprintf(fid, '            *Kp_out = pid_Kp[i];\n');
    fprintf(fid, '            *Ki_out = pid_Ki[i];\n');
    fprintf(fid, '            *Kd_out = pid_Kd[i];\n');
    fprintf(fid, '            return;\n');
    fprintf(fid, '        }\n\n');
    fprintf(fid, '        weights[i] = 1.0f / (dist * dist);  // IDW power = 2\n');
    fprintf(fid, '        total_weight += weights[i];\n');
    fprintf(fid, '    }\n\n');
    fprintf(fid, '    *Kp_out = 0.0f;\n');
    fprintf(fid, '    *Ki_out = 0.0f;\n');
    fprintf(fid, '    *Kd_out = 0.0f;\n\n');
    fprintf(fid, '    for (int i = 0; i < NUM_OPERATING_POINTS; i++) {\n');
    fprintf(fid, '        float w = weights[i] / total_weight;\n');
    fprintf(fid, '        *Kp_out += w * pid_Kp[i];\n');
    fprintf(fid, '        *Ki_out += w * pid_Ki[i];\n');
    fprintf(fid, '        *Kd_out += w * pid_Kd[i];\n');
    fprintf(fid, '    }\n');
    fprintf(fid, '}\n\n');
    
    % Funkcja aktualizacji CMSIS-DSP PID
    fprintf(fid, '// ============ Aktualizacja instancji CMSIS-DSP ============\n');
    fprintf(fid, 'static void update_cmsis_pid(arm_pid_instance_f32 *pid, \n');
    fprintf(fid, '                             float Kp, float Ki, float Kd,\n');
    fprintf(fid, '                             uint8_t reset_state)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    // Przelicz na współczynniki dyskretne CMSIS-DSP\n');
    fprintf(fid, '    // CMSIS używa: y[n] = y[n-1] + A0*e[n] + A1*e[n-1] + A2*e[n-2]\n');
    fprintf(fid, '    // Dla dyskretyzacji Tustin/Backward Euler:\n');
    fprintf(fid, '    //   Ki_discrete = Ki * Ts\n');
    fprintf(fid, '    //   Kd_discrete = Kd / Ts\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    float Ki_d = Ki * PID_SAMPLE_TIME_S;\n');
    fprintf(fid, '    float Kd_d = Kd / PID_SAMPLE_TIME_S;\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    pid->Kp = Kp;\n');
    fprintf(fid, '    pid->Ki = Ki_d;\n');
    fprintf(fid, '    pid->Kd = Kd_d;\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    // Oblicz współczynniki A0, A1, A2\n');
    fprintf(fid, '    pid->A0 = Kp + Ki_d + Kd_d;\n');
    fprintf(fid, '    pid->A1 = -Kp - 2.0f * Kd_d;\n');
    fprintf(fid, '    pid->A2 = Kd_d;\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    // Reset stanu jeśli wymagany\n');
    fprintf(fid, '    if (reset_state) {\n');
    fprintf(fid, '        pid->state[0] = 0.0f;  // e[n-1]\n');
    fprintf(fid, '        pid->state[1] = 0.0f;  // e[n-2]\n');
    fprintf(fid, '        pid->state[2] = 0.0f;  // y[n-1] (output)\n');
    fprintf(fid, '    }\n');
    fprintf(fid, '}\n\n');
    
    % Główne funkcje API
    fprintf(fid, '// ============ API publiczne ============\n\n');
    fprintf(fid, 'void PID_Init(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    float Kp, Ki, Kd;\n');
    fprintf(fid, '    interpolate_params(heater_pwm, fan_pwm, &Kp, &Ki, &Kd);\n');
    fprintf(fid, '    update_cmsis_pid(pid, Kp, Ki, Kd, 1);  // Reset state\n');
    fprintf(fid, '}\n\n');
    
    fprintf(fid, 'void PID_Interpolate(uint16_t heater_pwm, uint16_t fan_pwm,\n');
    fprintf(fid, '                     arm_pid_instance_f32 *pid, uint8_t reset_state)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    float Kp, Ki, Kd;\n');
    fprintf(fid, '    interpolate_params(heater_pwm, fan_pwm, &Kp, &Ki, &Kd);\n');
    fprintf(fid, '    update_cmsis_pid(pid, Kp, Ki, Kd, reset_state);\n');
    fprintf(fid, '}\n\n');
    
    fprintf(fid, 'void PID_GetNearest(uint16_t heater_pwm, uint16_t fan_pwm, arm_pid_instance_f32 *pid)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    float min_dist = 1e9f;\n');
    fprintf(fid, '    int nearest = 0;\n\n');
    fprintf(fid, '    for (int i = 0; i < NUM_OPERATING_POINTS; i++) {\n');
    fprintf(fid, '        float dh = (float)heater_pwm - (float)op_heater[i];\n');
    fprintf(fid, '        float df = (float)fan_pwm - (float)op_fan[i];\n');
    fprintf(fid, '        float dist = dh*dh + df*df;\n\n');
    fprintf(fid, '        if (dist < min_dist) {\n');
    fprintf(fid, '            min_dist = dist;\n');
    fprintf(fid, '            nearest = i;\n');
    fprintf(fid, '        }\n');
    fprintf(fid, '    }\n\n');
    fprintf(fid, '    update_cmsis_pid(pid, pid_Kp[nearest], pid_Ki[nearest], pid_Kd[nearest], 1);\n');
    fprintf(fid, '}\n');
    
    fclose(fid);
    
    %% ============ Przykład użycia ============
    fid = fopen('pid_example.c', 'w');
    
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @file pid_example.c\n');
    fprintf(fid, ' * @brief Przykład użycia PID z interpolacją i CMSIS-DSP\n');
    fprintf(fid, ' */\n\n');
    fprintf(fid, '#include "pid_lookup.h"\n');
    fprintf(fid, '#include "arm_math.h"\n\n');
    fprintf(fid, '// Instancja PID\n');
    fprintf(fid, 'static arm_pid_instance_f32 pid_temp;\n\n');
    fprintf(fid, '// Zmienne globalne\n');
    fprintf(fid, 'static uint16_t heater_pwm = 699;  // Aktualne PWM grzałki [0-999]\n');
    fprintf(fid, 'static uint16_t fan_pwm = 299;     // Aktualne PWM wentylatora [0-999]\n');
    fprintf(fid, 'static float temperature_setpoint = 35.0f;  // Zadana temperatura [°C]\n');
    fprintf(fid, 'static float output_min = 0.0f;    // Min wyjście PID\n');
    fprintf(fid, 'static float output_max = 999.0f;  // Max wyjście PID (PWM)\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Inicjalizacja systemu\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'void Temperature_Control_Init(void)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    // Inicjalizuj PID dla początkowego punktu pracy\n');
    fprintf(fid, '    PID_Init(heater_pwm, fan_pwm, &pid_temp);\n');
    fprintf(fid, '}\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Główna pętla regulacji (wywoływana co Ts = 0.5s)\n');
    fprintf(fid, ' * @param current_temp Aktualna temperatura [°C]\n');
    fprintf(fid, ' * @return Wyjście regulatora [0-999 PWM]\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'float Temperature_Control_Update(float current_temp)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    // 1. Aktualizuj parametry PID dla aktualnego punktu pracy\n');
    fprintf(fid, '    //    (bez resetowania stanu - zachowujemy historię błędów)\n');
    fprintf(fid, '    PID_Interpolate(heater_pwm, fan_pwm, &pid_temp, 0);\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    // 2. Oblicz błąd regulacji\n');
    fprintf(fid, '    float error = temperature_setpoint - current_temp;\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    // 3. Wywołaj CMSIS-DSP PID\n');
    fprintf(fid, '    float output = arm_pid_f32(&pid_temp, error);\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    // 4. Saturacja wyjścia do zakresu PWM\n');
    fprintf(fid, '    if (output > output_max) output = output_max;\n');
    fprintf(fid, '    if (output < output_min) output = output_min;\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    // 5. Aktualizuj PWM grzałki\n');
    fprintf(fid, '    heater_pwm = (uint16_t)output;\n');
    fprintf(fid, '    \n');
    fprintf(fid, '    return output;\n');
    fprintf(fid, '}\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Zmiana setpointu\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'void Temperature_Control_SetTarget(float target_temp)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    temperature_setpoint = target_temp;\n');
    fprintf(fid, '}\n\n');
    fprintf(fid, '/**\n');
    fprintf(fid, ' * @brief Ustawienie PWM wentylatora (ręczne lub z innego regulatora)\n');
    fprintf(fid, ' */\n');
    fprintf(fid, 'void Temperature_Control_SetFanPWM(uint16_t pwm)\n');
    fprintf(fid, '{\n');
    fprintf(fid, '    fan_pwm = (pwm > 999) ? 999 : pwm;\n');
    fprintf(fid, '}\n');
    
    fclose(fid);
    
    fprintf('Wygenerowano pliki:\n');
    fprintf('  - pid_lookup.h    (nagłówek z API)\n');
    fprintf('  - pid_lookup.c    (implementacja z CMSIS-DSP)\n');
    fprintf('  - pid_example.c   (przykład użycia)\n\n');
    
    % Wyświetl zawartość
    fprintf('--- pid_lookup.h ---\n');
    type('pid_lookup.h');
    fprintf('\n--- pid_lookup.c ---\n');
    type('pid_lookup.c');
    fprintf('\n--- pid_example.c ---\n');
    type('pid_example.c');
end