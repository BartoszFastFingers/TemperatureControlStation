 clear all;
clc;

portName = 'COM9';  
baudRate = 115200;  

time_duration_of_test = 10 *60;  
ts = 0.5;                      

outputFolder = 'data';
if ~exist(outputFolder, 'dir')
    mkdir (outputFolder);
end


csvFileName = fullfile(outputFolder, 'temperature_fan30_heater70_ data_filtered.csv');
maxSamples = floor(time_duration_of_test / ts);  

s = serialport(portName, baudRate);
configureTerminator(s, "CR/LF");
flush(s);

fprintf('Połączono z portem %s\n', portName);

timestamp = datetime.empty(0,1);
temperature = [];
time_ms = [];

sampleCount = 0;

fprintf('Rozpoczynam odczyt danych...\n');

while sampleCount < maxSamples
    % Odczytaj linię
    dataLine = readline(s); 
    
    % Parsowanie danych
    parsed = sscanf(dataLine, 'T: %f CC, CCR: %d time_ms');
    if numel(parsed) == 2
        temp = parsed(1);
        t_ms = parsed(2);
        
        sampleCount = sampleCount + 1;
        current_time = datetime('now');
        
        timestamp(end+1,1) = current_time;
        temperature(end+1,1) = temp;
        time_ms(end+1,1) = t_ms;
        
        fprintf('[%d] Czas: %s, Temperatura: %.2f°C, Time_ms: %d\n', ...
            sampleCount, datestr(current_time,'HH:MM:SS'), temp, t_ms);
    end
    
    % Poczekaj do następnej próbki
    pause(ts);
end

clear s;
fprintf('Port zamknięty.\n');

dataTable = table(timestamp, temperature, time_ms, ...
    'VariableNames', {'Timestamp', 'Temperature_C', 'Time_ms'});
writetable(dataTable, csvFileName);
fprintf('Zapisano %d próbek do pliku: %s\n', sampleCount, csvFileName);

figure;

subplot(2,1,1);
plot(timestamp, temperature, 'b-o', 'LineWidth', 1.5);
ylabel('Temperatura [°C]');
title('Pomiar temperatury w czasie');
grid on;

subplot(2,1,2);
plot(time_ms/1000, temperature, 'r-o', 'LineWidth', 1.5);
ylabel('Temperatura [°C]');
xlabel('Czas [s]');
title('Temperatura vs. Time_ms z STM32');
grid on;
