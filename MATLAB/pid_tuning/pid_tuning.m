identify_system2;
clc; clear; close all;

%% 1. Wczytanie zidentyfikowanych modeli
load('identified_models.mat');

%% 2. Projekt PID
pid_heater = design_pid_heater(model_heater);
pid_fan    = design_pid_fan(model_fan);

%% 3. Symulacja odpowiedzi skokowych
simulate_pid(model_heater, pid_heater, 'Grzałka');
simulate_pid(model_fan,    pid_fan,    'Wentylator');

%% 4. Zapis regulatorów
save('pid_controllers.mat', 'pid_heater', 'pid_fan');
