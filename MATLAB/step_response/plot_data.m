data = readtable('data/temperature_heater_data_unfiltered.csv');

time_ms = data.Time_ms;
temperature_C = data.Temperature_C;

figure;
plot(time_ms, temperature_C, '-o', 'Color', [0.85 0.33 0.1], 'MarkerSize', 4);
title('Temperatura elementu grzewczego w funkcji czasu');
xlabel('Czas [ms]');
ylabel('Temperatura [°C]');
grid on;
