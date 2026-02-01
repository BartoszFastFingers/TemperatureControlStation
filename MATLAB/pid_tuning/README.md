# PID Tuning z Interpolacją dla STM32

System automatycznego strojenia regulatora PID na podstawie odpowiedzi skokowych.
Obsługuje wiele punktów pracy (heater + fan) z interpolacją parametrów.

## Struktura folderów

```
MATLAB/
├── step_response/
│   └── data/                          # Dane pomiarowe
│       ├── temperature_heater_data_filtered.csv
│       ├── temperature_fan_ data_filtered.csv
│       ├── temperature_fan_heater_ data_filtered.csv
│       └── temperature_fan30_heater70_ data_filtered.csv
│
└── pid_tuning/                        # TEN FOLDER
    ├── main_pid_tuning.m              # Główny skrypt (uruchom to!)
    ├── identify_system.m              # Identyfikacja K, tau, theta
    ├── tune_pid.m                     # Strojenie Kp, Ki, Kd
    ├── interpolate_pid.m              # Interpolacja + generowanie kodu C
    ├── simulate_pid.m                 # Symulacja do weryfikacji
    └── README.md                      # Ten plik
```

## Szybki start

1. **Umieść dane pomiarowe** w `../step_response/data/`
2. **Uruchom główny skrypt:**
   ```matlab
   >> main_pid_tuning
   ```
3. **Skopiuj wygenerowane pliki** `pid_lookup.h` i `pid_lookup.c` do projektu STM32

## Format danych wejściowych

Pliki CSV z kolumnami:
```
Timestamp,Temperature_C,Time_ms
01-Feb-2026 00:54:54,21.02,0
01-Feb-2026 00:54:54,21.04,500
...
```

- Próbkowanie: 0.5s (500ms)
- Czas trwania: 15 minut (1800 próbek)

## Pomiary wymagane

| # | Heater | Fan | Cel |
|---|--------|-----|-----|
| 1 | 100% | 0% | Czyste grzanie |
| 2 | 0% | 100% | Czyste chłodzenie |
| 3 | 50% | 50% | Równowaga |
| 4 | 70% | 30% | Typowa praca |

## Użycie w STM32

```c
#include "pid_lookup.h"

// W pętli głównej:
PID_Params_t params;

// Interpoluj parametry dla aktualnego punktu pracy
PID_Interpolate(heater_duty_percent, fan_duty_percent, &params);

// Użyj interpolowanych parametrów
pid_controller.Kp = params.Kp;
pid_controller.Ki = params.Ki;
pid_controller.Kd = params.Kd;

// Oblicz sterowanie
float control = PID_Compute(&pid_controller, setpoint, temperature);
```

## Metody strojenia

W `tune_pid.m` możesz wybrać metodę:
- `ZN` - Ziegler-Nichols (agresywne)
- `CC` - Cohen-Coon (dla dużych opóźnień)
- `IMC` - Internal Model Control (domyślne, zalecane)
- `SIMC` - Skogestad IMC (konserwatywne, PI)

## Symulacja

Po uruchomieniu `main_pid_tuning.m` możesz przetestować nastawy:

```matlab
>> simulate_pid(35, 70, 30)  % setpoint=35°C, heater=70%, fan=30%
```

## Pliki wyjściowe

| Plik | Zawartość |
|------|-----------|
| `identification_results.mat` | K, tau, theta dla każdego pomiaru |
| `pid_params.mat` | Kp, Ki, Kd dla każdego pomiaru |
| `pid_interpolators.mat` | Obiekty interpolacji MATLAB |
| `pid_lookup.h` | Nagłówek C dla STM32 |
| `pid_lookup.c` | Implementacja interpolacji C |

## Algorytm interpolacji

Używamy **Inverse Distance Weighting (IDW)**:

```
w_i = 1 / dist(punkt_aktualny, punkt_pomiarowy_i)^2
param = Σ(w_i * param_i) / Σ(w_i)
```

Działa dobrze dla 4+ punktów pomiarowych.

## Troubleshooting

### "Plik nie istnieje"
Sprawdź nazwy plików w `identify_system.m` - muszą zgadzać się z Twoimi.

### Dziwne wartości K lub tau
Pierwsze próbki mogą zawierać artefakty. Zwiększ `config.skip_samples` w `identify_system.m`.

### Za agresywna/powolna regulacja
Zmień `tuning_config.aggressiveness` w `tune_pid.m`:
- `0.5` = agresywna (szybsza, więcej przeregulowania)
- `1.0` = standardowa
- `2.0` = konserwatywna (wolniejsza, stabilniejsza)

