clc;
clear;
close all;

%% Wczytanie danych
dataFolder = '../step_response/data';
filename = fullfile(dataFolder, 'temperature_data_unfiltered.csv');

if ~isfile(filename)
    error('Plik CSV nie istnieje: %s', filename);
end

data = readmatrix(filename);

% Kolumny: [Timestamp | Temperature | Time_ms]
temperature = data(:,2);
time_ms = data(:,3);

%% Parametry filtrów SMA FIR
N_values = [5, 8, 10];
num_filters = length(N_values);

MSEs = zeros(1, num_filters);
RMSEs = zeros(1, num_filters);
MAEs = zeros(1, num_filters);

figure; hold on;
plot(time_ms, temperature, '.', 'DisplayName','Original signal');

num_coefficients = cell(1, num_filters);
den_coefficients = 1;

for k = 1:num_filters
    N = N_values(k);

    coeffs = (1/N)*ones(1,N);
    num_coefficients{k} = coeffs;

    y = filter(coeffs, den_coefficients, temperature);
    
    MSEs(k) = mean((temperature - y).^2);
    RMSEs(k) = sqrt(MSEs(k));
    MAEs(k) = mean(abs(temperature - y));
    
    % Wykres filtrowanego sygnału
    plot(time_ms, y, '-x', 'DisplayName', ['SMA FIR N=', num2str(N)]);
end

xlabel('Time [ms]');
ylabel('Temperature [°C]');
title('Comparison of SMA FIR filters');
legend; grid on;

results_table = table(N_values', MSEs', RMSEs', MAEs', ...
    'VariableNames', {'N','MSE','RMSE','MAE'});
disp(results_table);

x = input('Choose your filter {1, 2, 3}: ', 's');
selectedFilter = str2double(x);
if selectedFilter >= 1 && selectedFilter <= num_filters
    generate_SMA('SMA1', num_coefficients{selectedFilter}, 1);
else
    disp('Invalid filter selection. Please choose a valid filter number.');
end

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
