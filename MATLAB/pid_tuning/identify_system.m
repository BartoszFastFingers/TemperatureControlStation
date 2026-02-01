%% IDENTIFY_SYSTEM - Identyfikacja parametrów systemu z odpowiedzi skokowej

clear; clc; close all;

%% Konfiguracja - DOSTOSUJ DO SWOICH DANYCH
config.Ts = 0.5;  % Okres próbkowania [s]
config.skip_samples = 20;  % Pomijanie pierwszych próbek (artefakty)
config.pwm_max = 999;  % Maksymalna wartość PWM

% Nazwy plików z danymi (tylko filtrowane)
config.files = {
    'temperature_heater_data_filtered.csv',           % 999 heater, 0 fan
    'temperature_fan_ data_filtered.csv',             % 0 heater, 999 fan
    'temperature_fan_heater_ data_filtered.csv',      % 499 heater, 499 fan
    'temperature_fan30_heater70_ data_filtered.csv'   % 699 heater, 299 fan
};

% Opisy pomiarów
config.labels = {
    'Heater 999, Fan 0',
    'Heater 0, Fan 999',
    'Heater 499, Fan 499',
    'Heater 699, Fan 299'
};

% Punkty pracy [heater_pwm, fan_pwm] - wartości PWM 0-999
config.operating_points = [
    999, 0;     % Pomiar 1
    0, 999;     % Pomiar 2
    499, 499;   % Pomiar 3
    699, 299    % Pomiar 4
];

% Ścieżka do danych
config.data_path = '../step_response/data/';

%% Wczytanie i analiza danych
num_experiments = length(config.files);
results = struct();

figure('Name', 'Odpowiedzi skokowe', 'Position', [100, 100, 1200, 800]);

for i = 1:num_experiments
    % Wczytaj dane
    filepath = fullfile(config.data_path, config.files{i});
    
    if ~isfile(filepath)
        warning('Plik nie istnieje: %s', filepath);
        continue;
    end
    
    data = readtable(filepath);
    
    % Wyodrębnij temperaturę i czas
    temp = data.Temperature_C;
    time_ms = data.Time_ms;
    time_s = time_ms / 1000;
    
    % Pomiń artefakty na początku
    idx_start = config.skip_samples + 1;
    temp = temp(idx_start:end);
    time_s = time_s(idx_start:end) - time_s(idx_start);
    
    % Identyfikacja parametrów metodą 63%
    [K, tau, theta, T0, Tinf] = identify_fopdt(time_s, temp);
    
    % Zapis wyników
    results(i).label = config.labels{i};
    results(i).heater = config.operating_points(i, 1);
    results(i).fan = config.operating_points(i, 2);
    results(i).K = K;
    results(i).tau = tau;
    results(i).theta = theta;
    results(i).T0 = T0;
    results(i).Tinf = Tinf;
    
    % Wykres
    subplot(2, 2, i);
    plot(time_s, temp, 'b-', 'LineWidth', 1.5); hold on;
    
    % Model FOPDT
    t_model = linspace(0, max(time_s), 1000);
    y_model = fopdt_response(t_model, K, tau, theta, T0);
    plot(t_model, y_model, 'r--', 'LineWidth', 1.5);
    
    % Zaznacz punkty charakterystyczne
    plot([0 max(time_s)], [T0 T0], 'g:', 'LineWidth', 1);
    plot([0 max(time_s)], [Tinf Tinf], 'g:', 'LineWidth', 1);
    T63 = T0 + 0.63 * (Tinf - T0);
    plot([theta+tau theta+tau], [T0 Tinf], 'm:', 'LineWidth', 1);
    
    xlabel('Czas [s]');
    ylabel('Temperatura [°C]');
    title(sprintf('%s\nK=%.3f, τ=%.1fs, θ=%.1fs', config.labels{i}, K, tau, theta));
    legend('Dane', 'Model FOPDT', 'T_0 / T_{∞}', '', 'τ', 'Location', 'best');
    grid on;
end

sgtitle('Identyfikacja systemu - odpowiedzi skokowe');

%% Wyświetl wyniki
fprintf('\n========== WYNIKI IDENTYFIKACJI ==========\n\n');
fprintf('%-25s | %8s | %8s | %8s | %8s | %8s\n', ...
    'Pomiar', 'K [°C/%]', 'tau [s]', 'theta [s]', 'T0 [°C]', 'Tinf [°C]');
fprintf('%s\n', repmat('-', 1, 80));

for i = 1:length(results)
    if isfield(results(i), 'K') && ~isempty(results(i).K)
        fprintf('%-25s | %8.4f | %8.2f | %8.2f | %8.2f | %8.2f\n', ...
            results(i).label, results(i).K, results(i).tau, ...
            results(i).theta, results(i).T0, results(i).Tinf);
    end
end

%% Zapisz wyniki do pliku
save('identification_results.mat', 'results', 'config');
fprintf('\nWyniki zapisane do: identification_results.mat\n');

%% ==================== FUNKCJE LOKALNE ====================

function [K, tau, theta, T0, Tinf] = identify_fopdt(t, y)
    % IDENTIFY_FOPDT - Identyfikacja modelu FOPDT metodą 2 punktów (28% i 63%)
    %
    % Model: G(s) = K * exp(-theta*s) / (tau*s + 1)
    
    % Temperatura początkowa i końcowa
    T0 = mean(y(1:min(10, length(y))));  % Średnia z pierwszych 10 próbek
    Tinf = mean(y(end-50:end));           % Średnia z ostatnich 50 próbek
    
    % Zmiana temperatury (dla normalizacji)
    delta_T = Tinf - T0;
    
    % Jeśli chłodzenie (delta_T < 0), obsłuż to
    if abs(delta_T) < 0.5
        warning('Zbyt mała zmiana temperatury!');
        K = 0; tau = 1; theta = 0;
        return;
    end
    
    % Znormalizowana odpowiedź
    y_norm = (y - T0) / delta_T;
    
    % Znajdź czas dla 28.3% i 63.2% odpowiedzi
    % Używamy prostego wyszukiwania indeksu zamiast interp1
    % (bo y_norm może nie być monotoniczne)
    
    % Dla 28.3%
    idx_28 = find(y_norm >= 0.283, 1, 'first');
    if isempty(idx_28)
        idx_28 = 1;
    end
    t28 = t(idx_28);
    
    % Dla 63.2%
    idx_63 = find(y_norm >= 0.632, 1, 'first');
    if isempty(idx_63)
        idx_63 = round(length(t) * 0.63);  % Fallback
    end
    t63 = t(idx_63);
    
    % Interpolacja liniowa dla dokładniejszego wyniku
    if idx_28 > 1
        % Interpoluj między próbkami
        y1 = y_norm(idx_28 - 1);
        y2 = y_norm(idx_28);
        t1 = t(idx_28 - 1);
        t2 = t(idx_28);
        if y2 ~= y1
            t28 = t1 + (0.283 - y1) * (t2 - t1) / (y2 - y1);
        end
    end
    
    if idx_63 > 1
        y1 = y_norm(idx_63 - 1);
        y2 = y_norm(idx_63);
        t1 = t(idx_63 - 1);
        t2 = t(idx_63);
        if y2 ~= y1
            t63 = t1 + (0.632 - y1) * (t2 - t1) / (y2 - y1);
        end
    end
    
    % Oblicz tau i theta (metoda dwóch punktów Smitha)
    tau = 1.5 * (t63 - t28);
    theta = t63 - tau;
    
    % Korekta dla theta < 0
    if theta < 0
        theta = 0;
    end
    
    % Wzmocnienie statyczne
    % K = delta_T / delta_PWM, gdzie delta_PWM = pwm_max (999)
    % Jednostka: °C na jednostkę PWM
    K = delta_T / 999;  % °C na PWM (0-999)
end

function y = fopdt_response(t, K, tau, theta, T0)
    % FOPDT_RESPONSE - Symulacja odpowiedzi modelu FOPDT na skok jednostkowy
    %
    % y(t) = T0 + K*100 * (1 - exp(-(t-theta)/tau)) dla t >= theta
    
    y = zeros(size(t));
    idx = t >= theta;
    y(idx) = T0 + K * 100 * (1 - exp(-(t(idx) - theta) / tau));
    y(~idx) = T0;
end