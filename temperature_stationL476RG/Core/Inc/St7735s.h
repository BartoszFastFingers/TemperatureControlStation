/**
 * @file ST7735s.h
 * @brief Interfejs do wyświetlacza ST7735 (funkcje graficzne i wyświetlanie temperatury)
 *
 * Plik zawiera definicje stałych, struktur i funkcji do obsługi wyświetlacza
 * ST7735 w formacie RGB565 oraz funkcje specjalne dla wyświetlania temperatury.
 *
 * @author 00ary
 * @date 31-Jan-2026
 */

#ifndef ST7735S_H_
#define ST7735S_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/** @def ST7735_CS_PORT Pin CS wyświetlacza */
#define ST7735_CS_PORT      LCD_CS_GPIO_Port
/** @def ST7735_CS_PIN Numer pinu CS */
#define ST7735_CS_PIN       LCD_CS_Pin
/** @def ST7735_DC_PORT Pin DC wyświetlacza */
#define ST7735_DC_PORT      LCD_DC_GPIO_Port
/** @def ST7735_DC_PIN Numer pinu DC */
#define ST7735_DC_PIN       LCD_DC_Pin
/** @def ST7735_RST_PORT Pin RST wyświetlacza */
#define ST7735_RST_PORT     LCD_RST_GPIO_Port
/** @def ST7735_RST_PIN Numer pinu RST */
#define ST7735_RST_PIN      LCD_RST_Pin

/** @def ST7735_WIDTH Szerokość ekranu */
#define ST7735_WIDTH        128
/** @def ST7735_HEIGHT Wysokość ekranu */
#define ST7735_HEIGHT       160

/** @def ST7735_XSTART Początkowy X (offset) */
#define ST7735_XSTART       2
/** @def ST7735_YSTART Początkowy Y (offset) */
#define ST7735_YSTART       1

/** @def ST7735_BLACK Kolor czarny w RGB565 */
#define ST7735_BLACK        0x0000
/** @def ST7735_WHITE Kolor biały w RGB565 */
#define ST7735_WHITE        0xFFFF
/** @def ST7735_RED Kolor czerwony w RGB565 */
#define ST7735_RED          0xF800
/** @def ST7735_GREEN Kolor zielony w RGB565 */
#define ST7735_GREEN        0x07E0
/** @def ST7735_BLUE Kolor niebieski w RGB565 */
#define ST7735_BLUE         0x001F
/** @def ST7735_YELLOW Kolor żółty w RGB565 */
#define ST7735_YELLOW       0xFFE0
/** @def ST7735_CYAN Kolor cyjan w RGB565 */
#define ST7735_CYAN         0x07FF
/** @def ST7735_MAGENTA Kolor magenta w RGB565 */
#define ST7735_MAGENTA      0xF81F
/** @def ST7735_ORANGE Kolor pomarańczowy w RGB565 */
#define ST7735_ORANGE       0xFC00
/** @def ST7735_GRAY Kolor szary w RGB565 */
#define ST7735_GRAY         0x8410
/** @def ST7735_DARKGRAY Kolor ciemnoszary w RGB565 */
#define ST7735_DARKGRAY     0x4208
/** @def ST7735_LIGHTGRAY Kolor jasnoszary w RGB565 */
#define ST7735_LIGHTGRAY    0xC618

/**
 * @brief Struktura uchwytu wyświetlacza ST7735
 */
typedef struct {
    SPI_HandleTypeDef *hspi; /**< Wskaźnik do SPI */
    uint16_t width;           /**< Szerokość ekranu */
    uint16_t height;          /**< Wysokość ekranu */
} ST7735_Handle_t;

/* ============ FUNKCJE PODSTAWOWE ============ */

/**
 * @brief Inicjalizacja wyświetlacza
 */
void ST7735_Init(ST7735_Handle_t *hst7735, SPI_HandleTypeDef *hspi);

/**
 * @brief Wypełnienie całego ekranu kolorem
 */
void ST7735_FillScreen(ST7735_Handle_t *hst7735, uint16_t color);

/**
 * @brief Rysowanie pojedynczego piksela
 */
void ST7735_DrawPixel(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Wypełnienie prostokąta kolorem
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
 */
void ST7735_DrawFloat(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      float num, uint8_t decimals, uint16_t color, uint16_t bg, uint8_t size);

/* ============ FUNKCJE SPECJALNE DLA TEMPERATURY ============ */

/**
 * @brief Wyświetlenie temperatury z dużą czcionką
 */
void ST7735_ShowTemperature(ST7735_Handle_t *hst7735, float temp, float setpoint,
                            uint16_t heater_pwm, uint16_t fan_pwm);

/**
 * @brief Rysowanie paska postępu (np. dla PWM)
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
