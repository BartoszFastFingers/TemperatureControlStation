/**
 * @file SMA1_fir.h
 * @brief Definicje i deklaracje dla prostego filtra uśredniającego (SMA) w formacie ARM CMSIS-DSP.
 *
 * Ten plik nagłówkowy został wygenerowany automatycznie przez MATLAB i zawiera:
 * - stałe konfiguracji filtra (liczba współczynników, rozmiar bloku),
 * - wskaźniki na stan filtra i współczynniki,
 * - deklarację instancji filtra CMSIS-DSP.
 *
 * @author MATLAB
 * @date 05-Feb-2026
 */

#ifndef INC_SMA1_H_
#define INC_SMA1_H_

#include "arm_math.h"

/**
 * @def SMA1_NUM_TAPS
 * @brief Liczba współczynników (tapów) filtra SMA1.
 */
#define SMA1_NUM_TAPS  10

/**
 * @def SMA1_BLOCK_SIZE
 * @brief Rozmiar bloku danych przetwarzanych w jednej iteracji filtra.
 */
#define SMA1_BLOCK_SIZE  1

/**
 * @brief Wskaźnik na tablicę przechowującą stan filtra FIR.
 *
 * Tablica dynamicznie alokowana i wykorzystywana przez funkcje CMSIS-DSP
 * do przechowywania poprzednich próbek filtra FIR.
 */
extern float32_t *SMA1_STATE;

/**
 * @brief Wskaźnik na tablicę współczynników filtra FIR.
 *
 * Tablica dynamicznie alokowana i wypełniana współczynnikami filtra.
 */
extern float32_t *SMA1_COEFFS;

/**
 * @brief Instancja filtra FIR typu float32 CMSIS-DSP.
 *
 * Struktura przechowująca stan filtra, liczbę tapów i wskaźniki na współczynniki.
 */
extern arm_fir_instance_f32 SMA1;

#endif // INC_SMA1_H_
