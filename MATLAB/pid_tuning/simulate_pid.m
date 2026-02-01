%% SIMULATE_PID - Symulacja odpowiedzi zamkniętej pętli z PID

function simulate_pid(setpoint, heater_duty, fan_duty)
    % SIMULATE_PID - Symuluje odpowiedź regulatora PID
    %
    % Argumenty:
    %   setpoint    - Zadana temperatura [°C]
    %   heater_duty - Wypełnienie grzałki [%]
    %   fan_duty    - Wypełnienie wentylatora [%]
    
    if nargin < 1
        setpoint = 35;  % Domyślnie 35°C
    end
    if nargin < 2
        heater_duty = 70;
    end
    if nargin < 3
        fan_duty = 30;
    end
    
    %% Wczytaj dane
    if ~isfile('pid_interpolators.mat') || ~isfile('identification_results.mat')
        error('Najpierw uruchom main_pid_tuning.m!');
    end
    
    load('pid_interpolators.mat', 'F_Kp', 'F_Ki', 'F_Kd');
    load('identification_results.mat', 'results');
    
    %% Interpoluj parametry PID
    Kp = F_Kp(heater_duty, fan_duty);
    Ki = F_Ki(heater_duty, fan_duty);
    Kd = F_Kd(heater_duty, fan_duty);
    
    fprintf('Symulacja dla punktu pracy: Heater=%d%%, Fan=%d%%\n', heater_duty, fan_duty);
    fprintf('Interpolowane parametry: Kp=%.4f, Ki=%.4f, Kd=%.4f\n', Kp, Ki, Kd);
    fprintf('Setpoint: %.1f°C\n\n', setpoint);
    
    %% Znajdź najbliższy zmierzony punkt pracy dla modelu
    distances = zeros(length(results), 1);
    for i = 1:length(results)
        distances(i) = sqrt((results(i).heater - heater_duty)^2 + ...
                           (results(i).fan - fan_duty)^2);
    end
    [~, idx] = min(distances);
    
    K = results(idx).K;
    tau = results(idx).tau;
    theta = results(idx).theta;
    T0 = results(idx).T0;
    
    fprintf('Używam modelu z pomiaru: %s\n', results(idx).label);
    fprintf('Model FOPDT: K=%.4f, tau=%.1fs, theta=%.1fs\n\n', K, tau, theta);
    
    %% Parametry symulacji
    Ts = 0.5;           % Okres próbkowania [s]
    t_sim = 300;        % Czas symulacji [s]
    t = 0:Ts:t_sim;
    n = length(t);
    
    %% Inicjalizacja
    y = zeros(n, 1);        % Temperatura
    u = zeros(n, 1);        % Sterowanie (PWM grzałki)
    e = zeros(n, 1);        % Błąd
    y(1) = T0;              % Początkowa temperatura
    
    % Stan PID
    integral = 0;
    prev_error = 0;
    
    % Bufor opóźnienia (dla theta)
    delay_samples = round(theta / Ts);
    u_delayed = zeros(delay_samples + 1, 1);
    
    %% Symulacja
    for k = 2:n
        % Błąd regulacji
        e(k) = setpoint - y(k-1);
        
        % PID
        integral = integral + e(k) * Ts;
        derivative = (e(k) - prev_error) / Ts;
        
        % Sterowanie
        u(k) = Kp * e(k) + Ki * integral + Kd * derivative;
        
        % Saturacja (0-100%)
        u(k) = max(0, min(100, u(k)));
        
        % Anti-windup
        if u(k) == 0 || u(k) == 100
            integral = integral - e(k) * Ts;  % Cofnij całkowanie
        end
        
        % Opóźnienie transportowe
        u_delayed = [u(k); u_delayed(1:end-1)];
        u_eff = u_delayed(end);
        
        % Model procesu (Euler)
        % dy/dt = (K*u - (y - T_ambient)) / tau
        T_ambient = T0;  % Zakładamy T0 jako temperaturę otoczenia
        dy = (K * u_eff * 100 - (y(k-1) - T_ambient)) / tau;
        y(k) = y(k-1) + dy * Ts;
        
        prev_error = e(k);
    end
    
    %% Metryki jakości
    % Czas narastania (10% do 90%)
    y_change = y - T0;
    y_ss = setpoint - T0;  % Zmiana w stanie ustalonym
    
    idx_10 = find(y_change >= 0.1 * y_ss, 1, 'first');
    idx_90 = find(y_change >= 0.9 * y_ss, 1, 'first');
    
    if ~isempty(idx_10) && ~isempty(idx_90)
        rise_time = t(idx_90) - t(idx_10);
    else
        rise_time = NaN;
    end
    
    % Przeregulowanie
    overshoot = (max(y) - setpoint) / (setpoint - T0) * 100;
    overshoot = max(0, overshoot);
    
    % Czas ustalania (2% pasmo)
    settling_band = 0.02 * abs(setpoint - T0);
    idx_settled = find(abs(y - setpoint) > settling_band, 1, 'last');
    if ~isempty(idx_settled) && idx_settled < n
        settling_time = t(idx_settled);
    else
        settling_time = t(end);
    end
    
    % Błąd ustalony
    steady_state_error = setpoint - mean(y(end-50:end));
    
    fprintf('═══════ METRYKI JAKOŚCI ═══════\n');
    fprintf('Czas narastania (10-90%%): %.1f s\n', rise_time);
    fprintf('Przeregulowanie:          %.1f %%\n', overshoot);
    fprintf('Czas ustalania (2%%):      %.1f s\n', settling_time);
    fprintf('Błąd ustalony:            %.2f °C\n', steady_state_error);
    fprintf('═══════════════════════════════\n');
    
    %% Wykres
    figure('Name', sprintf('Symulacja PID - H=%d%%, F=%d%%', heater_duty, fan_duty), ...
           'Position', [100, 100, 1000, 600]);
    
    subplot(2,1,1);
    plot(t, y, 'b-', 'LineWidth', 1.5); hold on;
    plot([0 t_sim], [setpoint setpoint], 'r--', 'LineWidth', 1);
    plot([0 t_sim], [setpoint-settling_band setpoint-settling_band], 'g:', 'LineWidth', 1);
    plot([0 t_sim], [setpoint+settling_band setpoint+settling_band], 'g:', 'LineWidth', 1);
    xlabel('Czas [s]');
    ylabel('Temperatura [°C]');
    title(sprintf('Odpowiedź regulatora PID\nKp=%.4f, Ki=%.4f, Kd=%.4f', Kp, Ki, Kd));
    legend('Temperatura', 'Setpoint', '±2% pasmo', 'Location', 'best');
    grid on;
    ylim([T0-1, max(y)+2]);
    
    subplot(2,1,2);
    plot(t, u, 'b-', 'LineWidth', 1.5);
    xlabel('Czas [s]');
    ylabel('Sterowanie [%]');
    title('Sygnał sterujący (PWM grzałki)');
    grid on;
    ylim([-5, 105]);
    
    sgtitle(sprintf('Symulacja: Heater=%d%%, Fan=%d%%, Setpoint=%.1f°C', ...
            heater_duty, fan_duty, setpoint));
end
