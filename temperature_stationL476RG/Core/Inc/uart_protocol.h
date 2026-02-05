/**
 * @file uart_protocol.h
 * @brief Protokół UART dla komunikacji z PC/innym urządzeniem (komendy, odczyt temperatury, PWM)
 *
 * Plik definiuje struktury, komendy i funkcje obsługujące protokół UART:
 * - obsługa komend (PING, SET TEMP, START/STOP, PID),
 * - wysyłanie danych pomiarowych,
 * - obliczanie CRC8,
 * - parsowanie odebranych bajtów.
 *
 * @author 00ary
 * @date 01-Feb-2026
 */

#ifndef CORE_INC_UART_PROTOCOL_H_
#define CORE_INC_UART_PROTOCOL_H_

#include "main.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

/** @def UART_PROTOCOL_HANDLE Domyślny uchwyt UART */
#define UART_PROTOCOL_HANDLE    huart2
/** @def UART_RX_BUFFER_SIZE Rozmiar bufora odbiorczego */
#define UART_RX_BUFFER_SIZE     128
/** @def UART_TX_BUFFER_SIZE Rozmiar bufora nadawczego */
#define UART_TX_BUFFER_SIZE     128

/**
 * @brief Lista dostępnych komend UART
 */
typedef enum {
    CMD_NONE = 0,       /**< Brak komendy */
    CMD_PING,           /**< $PING*XX */
    CMD_SET_TEMP,       /**< $SETP,40.0*XX */
    CMD_GET_STATUS,     /**< $STAT*XX */
    CMD_START,          /**< $START*XX */
    CMD_STOP,           /**< $STOP*XX */
    CMD_SET_KP,         /**< $SEKP,10.5*XX */
    CMD_SET_KI,         /**< $SEKI,0.5*XX */
    CMD_SET_KD,         /**< $SEKD,0.0*XX */
} UartCommand_t;

/**
 * @brief Struktura protokołu UART
 */
typedef struct {
    UART_HandleTypeDef *huart; /**< Wskaźnik do UART */

    // Bufor odbiorczy
    uint8_t rx_buffer[UART_RX_BUFFER_SIZE]; /**< Bufor przychodzący */
    uint16_t rx_index;                       /**< Indeks w buforze RX */
    bool rx_complete;                        /**< Flaga zakończenia odbioru */

    // Bufor nadawczy
    uint8_t tx_buffer[UART_TX_BUFFER_SIZE]; /**< Bufor nadawczy */

    // Parsowana komenda
    UartCommand_t last_command; /**< Ostatnia odebrana komenda */
    float command_arg;           /**< Argument komendy */

    // Status
    bool running;                /**< Flaga stanu systemu */
} UartProtocol_t;

/* ===================== FUNKCJE ===================== */

/**
 * @brief Inicjalizacja protokołu UART
 * @param proto Wskaźnik na strukturę protokołu
 * @param huart Uchwyt UART
 */
void UartProtocol_Init(UartProtocol_t *proto, UART_HandleTypeDef *huart);

/**
 * @brief Obliczenie CRC8 (polynomial 0x07)
 * @param data Dane wejściowe
 * @param len Długość danych
 * @return Wartość CRC8
 */
uint8_t UartProtocol_CRC8(const uint8_t *data, uint16_t len);

/**
 * @brief Wysłanie danych pomiarowych
 * @param proto Wskaźnik na strukturę protokołu
 * @param temp Aktualna temperatura
 * @param setpoint Temperatura zadana
 * @param heater_pwm PWM grzałki
 * @param fan_pwm PWM wentylatora
 * @param timestamp Znacznik czasu [ms]
 */
void UartProtocol_SendData(UartProtocol_t *proto,
                           float temp, float setpoint,
                           uint16_t heater_pwm, uint16_t fan_pwm,
                           uint32_t timestamp);

/**
 * @brief Wysłanie odpowiedzi PONG
 */
void UartProtocol_SendPong(UartProtocol_t *proto);

/**
 * @brief Wysłanie potwierdzenia (ACK)
 * @param proto Wskaźnik na strukturę protokołu
 * @param cmd Komenda która została potwierdzona
 */
void UartProtocol_SendAck(UartProtocol_t *proto, const char *cmd);

/**
 * @brief Wysłanie błędu (NAK)
 * @param proto Wskaźnik na strukturę protokołu
 * @param error Kod błędu
 */
void UartProtocol_SendNak(UartProtocol_t *proto, const char *error);

/**
 * @brief Przetworzenie odebranego bajtu (wywoływane z przerwania)
 * @param proto Wskaźnik na strukturę protokołu
 * @param byte Odebrany bajt
 */
void UartProtocol_ReceiveByte(UartProtocol_t *proto, uint8_t byte);

/**
 * @brief Parsowanie odebranej komendy (wywoływane z głównej pętli)
 * @param proto Wskaźnik na strukturę protokołu
 * @return Typ odebranej komendy (CMD_NONE jeśli brak)
 */
UartCommand_t UartProtocol_Parse(UartProtocol_t *proto);

/**
 * @brief Pobranie argumentu ostatniej komendy
 * @param proto Wskaźnik na strukturę protokołu
 * @return Argument jako float
 */
float UartProtocol_GetArg(UartProtocol_t *proto);

/**
 * @brief Weryfikacja CRC odebranej wiadomości
 * @param proto Wskaźnik na strukturę protokołu
 * @return true jeśli CRC poprawne
 */
bool UartProtocol_VerifyCRC(UartProtocol_t *proto);

#endif /* CORE_INC_UART_PROTOCOL_H_ */
