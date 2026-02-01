#ifndef ST7735S_H_
#define ST7735S_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>


#define ST7735_CS_PORT      LCD_CS_GPIO_Port
#define ST7735_CS_PIN       LCD_CS_Pin
#define ST7735_DC_PORT      LCD_DC_GPIO_Port
#define ST7735_DC_PIN       LCD_DC_Pin
#define ST7735_RST_PORT     LCD_RST_GPIO_Port
#define ST7735_RST_PIN      LCD_RST_Pin

#define ST7735_WIDTH        128
#define ST7735_HEIGHT       160

#define ST7735_XSTART       2
#define ST7735_YSTART       1


#define ST7735_BLACK        0x0000
#define ST7735_WHITE        0xFFFF
#define ST7735_RED          0xF800
#define ST7735_GREEN        0x07E0
#define ST7735_BLUE         0x001F
#define ST7735_YELLOW       0xFFE0
#define ST7735_CYAN         0x07FF
#define ST7735_MAGENTA      0xF81F
#define ST7735_ORANGE       0xFC00
#define ST7735_GRAY         0x8410
#define ST7735_DARKGRAY     0x4208
#define ST7735_LIGHTGRAY    0xC618


typedef struct {
    SPI_HandleTypeDef *hspi;
    uint16_t width;
    uint16_t height;
} ST7735_Handle_t;

/* ============ FUNKCJE PODSTAWOWE ============ */

/**
 * @brief Inicjalizacja wyświetlacza
 * @param hst7735 Uchwyt wyświetlacza
 * @param hspi Uchwyt SPI
 */
void ST7735_Init(ST7735_Handle_t *hst7735, SPI_HandleTypeDef *hspi);

/**
 * @brief Wypełnienie całego ekranu kolorem
 * @param hst7735 Uchwyt wyświetlacza
 * @param color Kolor RGB565
 */
void ST7735_FillScreen(ST7735_Handle_t *hst7735, uint16_t color);

/**
 * @brief Rysowanie pojedynczego piksela
 */
void ST7735_DrawPixel(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Wypełnienie prostokąta
 */
void ST7735_FillRect(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Rysowanie prostokąta (tylko ramka)
 */
void ST7735_DrawRect(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Rysowanie linii poziomej
 */
void ST7735_DrawHLine(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      uint16_t w, uint16_t color);

/**
 * @brief Rysowanie linii pionowej
 */
void ST7735_DrawVLine(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      uint16_t h, uint16_t color);

/* ============ FUNKCJE TEKSTOWE ============ */

/**
 * @brief Wypisanie znaku
 * @param hst7735 Uchwyt wyświetlacza
 * @param x Pozycja X
 * @param y Pozycja Y
 * @param c Znak do wypisania
 * @param color Kolor znaku
 * @param bg Kolor tła
 * @param size Rozmiar (1 = 6x8, 2 = 12x16, 3 = 18x24)
 */
void ST7735_DrawChar(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                     char c, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Wypisanie stringa
 */
void ST7735_DrawString(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                       const char *str, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Wypisanie liczby całkowitej
 */
void ST7735_DrawInt(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                    int32_t num, uint16_t color, uint16_t bg, uint8_t size);

/**
 * @brief Wypisanie liczby zmiennoprzecinkowej
 * @param decimals Liczba miejsc po przecinku
 */
void ST7735_DrawFloat(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      float num, uint8_t decimals, uint16_t color, uint16_t bg, uint8_t size);

/* ============ FUNKCJE SPECJALNE DLA TEMPERATURY ============ */

/**
 * @brief Wyświetlenie temperatury z dużą czcionką
 * @param hst7735 Uchwyt wyświetlacza
 * @param temp Temperatura w °C
 * @param setpoint Temperatura zadana
 * @param heater_pwm PWM grzałki
 * @param fan_pwm PWM wentylatora
 */
void ST7735_ShowTemperature(ST7735_Handle_t *hst7735, float temp, float setpoint,
                            uint16_t heater_pwm, uint16_t fan_pwm);

/**
 * @brief Rysowanie paska postępu (np. dla PWM)
 * @param value Wartość 0-100 lub 0-999
 * @param max_value Maksymalna wartość
 */
void ST7735_DrawProgressBar(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h, uint16_t value, uint16_t max_value,
                            uint16_t bar_color, uint16_t bg_color);

/**
 * @brief Konwersja RGB na RGB565
 */
uint16_t ST7735_RGB(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Kolor zależny od temperatury (niebieski->zielony->czerwony)
 */
uint16_t ST7735_TempColor(float temp, float min_temp, float max_temp);

#endif /* ST7735S_H_ */
