/*
 * uart_protocol.h
 *
 *  Created on: 1 lut 2026
 *      Author: 00ary
 */

#ifndef CORE_INC_UART_PROTOCOL_H_
#define CORE_INC_UART_PROTOCOL_H_

#include "main.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>


#define UART_PROTOCOL_HANDLE    huart2
#define UART_RX_BUFFER_SIZE     128
#define UART_TX_BUFFER_SIZE     128


typedef enum {
    CMD_NONE = 0,
    CMD_PING,           // $PING*XX
    CMD_SET_TEMP,       // $SETP,40.0*XX
    CMD_GET_STATUS,     // $STAT*XX
    CMD_START,          // $START*XX
    CMD_STOP,           // $STOP*XX
    CMD_SET_KP,         // $SEKP,10.5*XX
    CMD_SET_KI,         // $SEKI,0.5*XX
    CMD_SET_KD,         // $SEKD,0.0*XX
} UartCommand_t;


typedef struct {
    UART_HandleTypeDef *huart;

    // Bufor odbiorczy
    uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
    uint16_t rx_index;
    bool rx_complete;

    // Bufor nadawczy
    uint8_t tx_buffer[UART_TX_BUFFER_SIZE];

    // Parsowana komenda
    UartCommand_t last_command;
    float command_arg;

    // Status
    bool running;

} UartProtocol_t;


/**
 * @brief Inicjalizacja protokołu
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
