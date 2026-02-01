%% TUNE_PID - Strojenie regulatora PID na podstawie zidentyfikowanych parametrów

clear; clc; close all;

%% Wczytaj wyniki identyfikacji
if ~isfile('identification_results.mat')
    error('Najpierw uruchom identify_system.m!');
end

load('identification_results.mat', 'results', 'config');

%% Konfiguracja strojenia
tuning_config.method = 'SIMC';  % 'ZN', 'CC', 'IMC', 'SIMC'
tuning_config.lambda_factor = 1.0;  % Dla IMC: lambda = lambda_factor * tau

% Aggressive (0.5), Moderate (1.0), Conservative (2.0)
tuning_config.aggressiveness = 3.0;  % Bardziej konserwatywne - mniej oscylacji

% Pomiń pomiary z bardzo małą zmianą temperatury (|ΔT| < próg)
tuning_config.min_delta_T = 2.0;  % °C - pomiar musi mieć min 2°C zmiany

%% Oblicz parametry PID dla każdego punktu pracy
num_points = length(results);
pid_params = struct('label', {}, 'heater', {}, 'fan', {}, 'Kp', {}, 'Ki', {}, 'Kd', {}, 'Ti', {}, 'Td', {}, 'lambda', {});
param_count = 0;

fprintf('\n========== STROJENIE PID - Metoda: %s ==========\n\n', tuning_config.method);
fprintf('%-25s | %8s | %8s | %8s | %8s | %8s | %8s\n', ...
    'Pomiar', 'Kp', 'Ki', 'Kd', 'Ti [s]', 'Td [s]', 'Lambda');
fprintf('%s\n', repmat('-', 1, 90));

for i = 1:num_points
    if ~isfield(results(i), 'K') || isempty(results(i).K)
        continue;
    end
    
    K = results(i).K;
    tau = results(i).tau;
    theta = results(i).theta;
    
    % Sprawdź czy zmiana temperatury jest wystarczająca
    delta_T = abs(results(i).Tinf - results(i).T0);
    if delta_T < tuning_config.min_delta_T
        fprintf('%-25s | POMINIĘTY (ΔT=%.1f°C < %.1f°C)\n', ...
            results(i).label, delta_T, tuning_config.min_delta_T);
        continue;
    end
    
    % Dobierz metodę strojenia
    switch upper(tuning_config.method)
        case 'ZN'  % Ziegler-Nichols
            [Kp, Ti, Td] = tune_ziegler_nichols(K, tau, theta);
            lambda = NaN;
            
        case 'CC'  % Cohen-Coon
            [Kp, Ti, Td] = tune_cohen_coon(K, tau, theta);
            lambda = NaN;
            
        case 'IMC'  % Internal Model Control
            lambda = tuning_config.lambda_factor * tau * tuning_config.aggressiveness;
            [Kp, Ti, Td] = tune_imc(K, tau, theta, lambda);
            
        case 'SIMC'  % Skogestad IMC
            [Kp, Ti, Td] = tune_simc(K, tau, theta);
            lambda = max(theta, 0.25*tau);
            
        otherwise
            error('Nieznana metoda: %s', tuning_config.method);
    end
    
    % Oblicz Ki i Kd
    Ki = Kp / Ti;
    Kd = Kp * Td;
    
    % Zapisz wyniki (bez luk w indeksach)
    param_count = param_count + 1;
    pid_params(param_count).label = results(i).label;
    pid_params(param_count).heater = results(i).heater;
    pid_params(param_count).fan = results(i).fan;
    pid_params(param_count).Kp = Kp;
    pid_params(param_count).Ki = Ki;
    pid_params(param_count).Kd = Kd;
    pid_params(param_count).Ti = Ti;
    pid_params(param_count).Td = Td;
    pid_params(param_count).lambda = lambda;
    
    % Wyświetl
    fprintf('%-25s | %8.4f | %8.4f | %8.4f | %8.2f | %8.2f | %8.2f\n', ...
        results(i).label, Kp, Ki, Kd, Ti, Td, lambda);
end

%% Zapisz parametry PID
save('pid_params.mat', 'pid_params', 'tuning_config');
fprintf('\nParametry PID zapisane do: pid_params.mat\n');

%% Wykres porównawczy
figure('Name', 'Parametry PID dla różnych punktów pracy', 'Position', [100, 100, 1000, 600]);

% Filtruj tylko wypełnione wpisy
valid_idx = [];
for i = 1:length(pid_params)
    if isfield(pid_params(i), 'Kp') && ~isempty(pid_params(i).Kp)
        valid_idx = [valid_idx, i];
    end
end

if isempty(valid_idx)
    warning('Brak prawidłowych parametrów PID do wykreślenia!');
    return;
end

% Przygotuj dane do wykresów (tylko valid)
labels = {pid_params(valid_idx).label};
heater_pct = [pid_params(valid_idx).heater];
fan_pct = [pid_params(valid_idx).fan];
Kp_vals = [pid_params(valid_idx).Kp];
Ki_vals = [pid_params(valid_idx).Ki];
Kd_vals = [pid_params(valid_idx).Kd];

% Wykres słupkowy
subplot(2,2,1);
bar(categorical(labels), Kp_vals);
ylabel('Kp');
title('Wzmocnienie proporcjonalne');
grid on;

subplot(2,2,2);
bar(categorical(labels), Ki_vals);
ylabel('Ki');
title('Wzmocnienie całkujące');
grid on;

subplot(2,2,3);
bar(categorical(labels), Kd_vals);
ylabel('Kd');
title('Wzmocnienie różniczkujące');
grid on;

subplot(2,2,4);
scatter3(heater_pct, fan_pct, Kp_vals, 100, 'filled');
xlabel('Heater [%]');
ylabel('Fan [%]');
zlabel('Kp');
title('Kp vs punkt pracy');
grid on;

sgtitle(sprintf('Parametry PID - Metoda: %s', tuning_config.method));

%% ==================== FUNKCJE STROJENIA ====================

function [Kp, Ti, Td] = tune_ziegler_nichols(K, tau, theta)
    % TUNE_ZIEGLER_NICHOLS - Metoda Ziegler-Nichols (odpowiedź skokowa)
    % Dla regulatora PID
    
    if theta < 0.01
        theta = 0.01;  % Minimalne opóźnienie
    end
    
    Kp = 1.2 * tau / (K * theta);
    Ti = 2 * theta;
    Td = 0.5 * theta;
end

function [Kp, Ti, Td] = tune_cohen_coon(K, tau, theta)
    % TUNE_COHEN_COON - Metoda Cohen-Coon
    % Lepsza dla procesów z większym opóźnieniem
    
    if theta < 0.01
        theta = 0.01;
    end
    
    r = theta / tau;  % Stosunek opóźnienia do stałej czasowej
    
    Kp = (1/(K*r)) * (4/3 + r/4);
    Ti = theta * (32 + 6*r) / (13 + 8*r);
    Td = theta * 4 / (11 + 2*r);
end

function [Kp, Ti, Td] = tune_imc(K, tau, theta, lambda)
    % TUNE_IMC - Internal Model Control
    % lambda - pożądana stała czasowa zamkniętej pętli
    
    if lambda < theta
        lambda = theta;  % Minimum to opóźnienie
    end
    
    Kp = (2*tau + theta) / (K * (2*lambda + theta));
    Ti = tau + theta/2;
    Td = tau * theta / (2*tau + theta);
end

function [Kp, Ti, Td] = tune_simc(K, tau, theta)
    % TUNE_SIMC - Skogestad IMC (proste reguły)
    % Domyślnie: lambda = max(theta, 0.25*tau)
    % Daje stabilne, konserwatywne nastawy
    
    lambda = max(theta, 0.25*tau);  
    
    % PI controller (Skogestad zaleca PI dla większości procesów termicznych)
    Kp = tau / (K * (lambda + theta));
    Ti = min(tau, 4*(lambda + theta));
    
    % Dla regulacji temperatury - PI zazwyczaj wystarcza
    % Ustaw Td = 0 dla czystego PI, lub bardzo małe dla odrobiny predykcji
    Td = 0;  % Czyste PI - stabilniejsze dla systemów termicznych
    
    % Jeśli chcesz PID, odkomentuj poniższe (ale ostrożnie!):
    % Td = theta / 4;  % Bardzo małe Td
end