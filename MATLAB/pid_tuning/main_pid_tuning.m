%% MAIN_PID_TUNING - Główny skrypt do strojenia PID z interpolacją


clear; clc; close all;

fprintf('╔════════════════════════════════════════════════════════════╗\n');
fprintf('║       SYSTEM STROJENIA PID Z INTERPOLACJĄ                 ║\n');
fprintf('║       dla regulacji temperatury (heater + fan)            ║\n');
fprintf('╚════════════════════════════════════════════════════════════╝\n\n');

%% Krok 1: Identyfikacja systemu
fprintf('▶ KROK 1: Identyfikacja systemu z odpowiedzi skokowych...\n');
fprintf('─────────────────────────────────────────────────────────\n');
run('identify_system.m');

fprintf('\n✓ Identyfikacja zakończona\n');
fprintf('  Wyniki w: identification_results.mat\n\n');

pause(1);

%% Krok 2: Strojenie PID
fprintf('▶ KROK 2: Strojenie regulatora PID...\n');
fprintf('─────────────────────────────────────────────────────────\n');
run('tune_pid.m');

fprintf('\n✓ Strojenie zakończone\n');
fprintf('  Parametry w: pid_params.mat\n\n');

pause(1);

%% Krok 3: Interpolacja
fprintf('▶ KROK 3: Tworzenie interpolatora i generowanie kodu C...\n');
fprintf('─────────────────────────────────────────────────────────\n');
run('interpolate_pid.m');

fprintf('\n✓ Interpolacja zakończona\n');
fprintf('  Interpolatory w: pid_interpolators.mat\n');
fprintf('  Kod C w: pid_lookup.h, pid_lookup.c\n\n');

%% Podsumowanie
fprintf('╔════════════════════════════════════════════════════════════╗\n');
fprintf('║                    ZAKOŃCZONO POMYŚLNIE                   ║\n');
fprintf('╚════════════════════════════════════════════════════════════╝\n\n');

fprintf('WYGENEROWANE PLIKI:\n');
fprintf('  1. identification_results.mat - parametry K, tau, theta\n');
fprintf('  2. pid_params.mat            - parametry Kp, Ki, Kd\n');
fprintf('  3. pid_interpolators.mat     - interpolatory MATLAB\n');
fprintf('  4. pid_lookup.h/.c           - kod C dla STM32\n\n');

fprintf('NASTĘPNE KROKI:\n');
fprintf('  1. Skopiuj pid_lookup.h i pid_lookup.c do projektu STM32\n');
fprintf('  2. W głównej pętli wywołuj PID_Interpolate() przed obliczeniem PID\n');
fprintf('  3. Przykład użycia w STM32:\n\n');

fprintf('     PID_Params_t params;\n');
fprintf('     PID_Interpolate(heater_duty, fan_duty, &params);\n');
fprintf('     pid.Kp = params.Kp;\n');
fprintf('     pid.Ki = params.Ki;\n');
fprintf('     pid.Kd = params.Kd;\n');
fprintf('     float output = PID_Compute(&pid, setpoint, temperature);\n\n');

srcDest = fullfile('../../temperature_stationL476RG/Components/Src');
incDest = fullfile('../../temperature_stationL476RG/Components/Inc');

if ~exist(srcDest, 'dir')
    mkdir(srcDest);
end
if ~exist(incDest, 'dir')
    mkdir(incDest);
end

cFiles = dir('*.c');
csvFiles = dir('*.csv');
for i = 1:length(cFiles)
    movefile(cFiles(i).name, fullfile(srcDest, cFiles(i).name));
end
for i = 1:length(csvFiles)
    movefile(csvFiles(i).name, fullfile(srcDest, csvFiles(i).name));
end

hFiles = dir('*.h');
for i = 1:length(hFiles)
    movefile(hFiles(i).name, fullfile(incDest, hFiles(i).name));
end