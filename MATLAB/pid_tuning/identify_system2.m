%% PID Tuning - Identification of Heater and Fan
% Author: 00ary
% Date: 05-Feb-2026

clc; clear; close all;

%% 1. Ścieżki do plików CSV
heater_file = fullfile('..','step_response','data','temperature_heater_data_unfiltered.csv');
fan_file    = fullfile('..','step_response','data','temperature_fan_ data_filtered.csv');

%% 2. Wczytanie danych
heater_data = readtable(heater_file);
fan_data    = readtable(fan_file);

%% 3. Usunięcie pierwszych 10 próbek (czujnik się stabilizuje)
heater_data(1:10,:) = [];
fan_data(1:10,:)    = [];

%% 4. Przygotowanie sygnałów
% Czas w sekundach
t_heater = heater_data.Time_ms / 1000;
t_fan    = fan_data.Time_ms / 1000;

% Wyjście systemu (temperatura)
y_heater = heater_data.Temperature_C;
y_fan    = fan_data.Temperature_C;

% Wejście z wstępnym okresem równowagi
nx = 5; % liczba próbek równowagi
u_heater = [zeros(nx,1); 1000*ones(length(y_heater)-nx,1)];
y_heater = [y_heater(1)*ones(nx,1); y_heater(nx+1:end)];

u_fan = [zeros(nx,1); 1000*ones(length(y_fan)-nx,1)]; 
y_fan = [y_fan(1)*ones(nx,1); y_fan(nx+1:end)];
%% 5. Utworzenie obiektów iddata
Ts_heater = mean(diff(t_heater));
Ts_fan    = mean(diff(t_fan));

data_heater = iddata(y_heater, u_heater, Ts_heater);
data_fan    = iddata(y_fan, u_fan, Ts_fan);

%% 6. Identyfikacja modeli 1. rzędu (bez opóźnienia początkowego)
model_heater = tfest(data_heater, 1, 0); 
model_fan    = tfest(data_fan, 1, 0);

%% 7. Wyciągnięcie parametrów
[num_h, den_h] = tfdata(model_heater, 'v'); 
K_heater = num_h(end);          % poprawne wzmocnienie
tau_heater = den_h(2);          % stała czasowa
Td_heater = model_heater.InputDelay;

[num_f, den_f] = tfdata(model_fan, 'v');
K_fan = num_f(end);             % poprawne wzmocnienie
tau_fan = den_f(2);
Td_fan = model_fan.InputDelay;

%% 8. Wyświetlenie parametrów
fprintf('--- Parametry grzałki ---\n');
fprintf('Wzmocnienie K = %.6f °C / PWM\n', K_heater);
fprintf('Stała czasowa tau = %.3f s\n', tau_heater);
fprintf('Opóźnienie Td = %.3f s\n', Td_heater);

fprintf('\n--- Parametry wentylatora ---\n');
fprintf('Wzmocnienie K_fan = %.6f °C / PWM\n', K_fan);
fprintf('Stała czasowa tau = %.3f s\n', tau_fan);
fprintf('Opóźnienie Td = %.3f s\n', Td_fan);

%% 9. Porównanie modelu z danymi
figure;
subplot(2,1,1)
compare(data_heater, model_heater);
title('Dopasowanie modelu grzałki');

subplot(2,1,2)
compare(data_fan, model_fan);
title('Dopasowanie modelu wentylatora (odwrócone wejście)');

%% 10. Zapis modeli do pliku
save('identified_models.mat', 'model_heater', 'model_fan', ...
     'K_heater', 'tau_heater', 'Td_heater', ...
     'K_fan', 'tau_fan', 'Td_fan');
