/*
 * St7735s.c
 *
 *  Created on: 1 lut 2026
 *      Author: 00ary
 */


#include "st7735s.h"
#include <string.h>
#include <stdio.h>


#define ST7735_NOP      0x00
#define ST7735_SWRESET  0x01
#define ST7735_SLPIN    0x10
#define ST7735_SLPOUT   0x11
#define ST7735_PTLON    0x12
#define ST7735_NORON    0x13
#define ST7735_INVOFF   0x20
#define ST7735_INVON    0x21
#define ST7735_DISPOFF  0x28
#define ST7735_DISPON   0x29
#define ST7735_CASET    0x2A
#define ST7735_RASET    0x2B
#define ST7735_RAMWR    0x2C
#define ST7735_RAMRD    0x2E
#define ST7735_COLMOD   0x3A
#define ST7735_MADCTL   0x36
#define ST7735_FRMCTR1  0xB1
#define ST7735_FRMCTR2  0xB2
#define ST7735_FRMCTR3  0xB3
#define ST7735_INVCTR   0xB4
#define ST7735_PWCTR1   0xC0
#define ST7735_PWCTR2   0xC1
#define ST7735_PWCTR3   0xC2
#define ST7735_PWCTR4   0xC3
#define ST7735_PWCTR5   0xC4
#define ST7735_VMCTR1   0xC5
#define ST7735_GMCTRP1  0xE0
#define ST7735_GMCTRN1  0xE1


static const uint8_t font5x7[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, // (space)
    0x00, 0x00, 0x5F, 0x00, 0x00, // !
    0x00, 0x07, 0x00, 0x07, 0x00, // "
    0x14, 0x7F, 0x14, 0x7F, 0x14, // #
    0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
    0x23, 0x13, 0x08, 0x64, 0x62, // %
    0x36, 0x49, 0x55, 0x22, 0x50, // &
    0x00, 0x05, 0x03, 0x00, 0x00, // '
    0x00, 0x1C, 0x22, 0x41, 0x00, // (
    0x00, 0x41, 0x22, 0x1C, 0x00, // )
    0x08, 0x2A, 0x1C, 0x2A, 0x08, // *
    0x08, 0x08, 0x3E, 0x08, 0x08, // +
    0x00, 0x50, 0x30, 0x00, 0x00, // ,
    0x08, 0x08, 0x08, 0x08, 0x08, // -
    0x00, 0x60, 0x60, 0x00, 0x00, // .
    0x20, 0x10, 0x08, 0x04, 0x02, // /
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1
    0x42, 0x61, 0x51, 0x49, 0x46, // 2
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4
    0x27, 0x45, 0x45, 0x45, 0x39, // 5
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6
    0x01, 0x71, 0x09, 0x05, 0x03, // 7
    0x36, 0x49, 0x49, 0x49, 0x36, // 8
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9
    0x00, 0x36, 0x36, 0x00, 0x00, // :
    0x00, 0x56, 0x36, 0x00, 0x00, // ;
    0x00, 0x08, 0x14, 0x22, 0x41, // <
    0x14, 0x14, 0x14, 0x14, 0x14, // =
    0x41, 0x22, 0x14, 0x08, 0x00, // >
    0x02, 0x01, 0x51, 0x09, 0x06, // ?
    0x32, 0x49, 0x79, 0x41, 0x3E, // @
    0x7E, 0x11, 0x11, 0x11, 0x7E, // A
    0x7F, 0x49, 0x49, 0x49, 0x36, // B
    0x3E, 0x41, 0x41, 0x41, 0x22, // C
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D
    0x7F, 0x49, 0x49, 0x49, 0x41, // E
    0x7F, 0x09, 0x09, 0x01, 0x01, // F
    0x3E, 0x41, 0x41, 0x51, 0x32, // G
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H
    0x00, 0x41, 0x7F, 0x41, 0x00, // I
    0x20, 0x40, 0x41, 0x3F, 0x01, // J
    0x7F, 0x08, 0x14, 0x22, 0x41, // K
    0x7F, 0x40, 0x40, 0x40, 0x40, // L
    0x7F, 0x02, 0x04, 0x02, 0x7F, // M
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O
    0x7F, 0x09, 0x09, 0x09, 0x06, // P
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q
    0x7F, 0x09, 0x19, 0x29, 0x46, // R
    0x46, 0x49, 0x49, 0x49, 0x31, // S
    0x01, 0x01, 0x7F, 0x01, 0x01, // T
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V
    0x7F, 0x20, 0x18, 0x20, 0x7F, // W
    0x63, 0x14, 0x08, 0x14, 0x63, // X
    0x03, 0x04, 0x78, 0x04, 0x03, // Y
    0x61, 0x51, 0x49, 0x45, 0x43, // Z
    0x00, 0x00, 0x7F, 0x41, 0x41, // [
    0x02, 0x04, 0x08, 0x10, 0x20, // backslash
    0x41, 0x41, 0x7F, 0x00, 0x00, // ]
    0x04, 0x02, 0x01, 0x02, 0x04, // ^
    0x40, 0x40, 0x40, 0x40, 0x40, // _
    0x00, 0x01, 0x02, 0x04, 0x00, // `
    0x20, 0x54, 0x54, 0x54, 0x78, // a
    0x7F, 0x48, 0x44, 0x44, 0x38, // b
    0x38, 0x44, 0x44, 0x44, 0x20, // c
    0x38, 0x44, 0x44, 0x48, 0x7F, // d
    0x38, 0x54, 0x54, 0x54, 0x18, // e
    0x08, 0x7E, 0x09, 0x01, 0x02, // f
    0x08, 0x14, 0x54, 0x54, 0x3C, // g
    0x7F, 0x08, 0x04, 0x04, 0x78, // h
    0x00, 0x44, 0x7D, 0x40, 0x00, // i
    0x20, 0x40, 0x44, 0x3D, 0x00, // j
    0x00, 0x7F, 0x10, 0x28, 0x44, // k
    0x00, 0x41, 0x7F, 0x40, 0x00, // l
    0x7C, 0x04, 0x18, 0x04, 0x78, // m
    0x7C, 0x08, 0x04, 0x04, 0x78, // n
    0x38, 0x44, 0x44, 0x44, 0x38, // o
    0x7C, 0x14, 0x14, 0x14, 0x08, // p
    0x08, 0x14, 0x14, 0x18, 0x7C, // q
    0x7C, 0x08, 0x04, 0x04, 0x08, // r
    0x48, 0x54, 0x54, 0x54, 0x20, // s
    0x04, 0x3F, 0x44, 0x40, 0x20, // t
    0x3C, 0x40, 0x40, 0x20, 0x7C, // u
    0x1C, 0x20, 0x40, 0x20, 0x1C, // v
    0x3C, 0x40, 0x30, 0x40, 0x3C, // w
    0x44, 0x28, 0x10, 0x28, 0x44, // x
    0x0C, 0x50, 0x50, 0x50, 0x3C, // y
    0x44, 0x64, 0x54, 0x4C, 0x44, // z
    0x00, 0x08, 0x36, 0x41, 0x00, // {
    0x00, 0x00, 0x7F, 0x00, 0x00, // |
    0x00, 0x41, 0x36, 0x08, 0x00, // }
    0x08, 0x08, 0x2A, 0x1C, 0x08, // ->
    0x08, 0x1C, 0x2A, 0x08, 0x08, // <-
};


static inline void ST7735_Select(void) {
    HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_RESET);
}

static inline void ST7735_Unselect(void) {
    HAL_GPIO_WritePin(ST7735_CS_PORT, ST7735_CS_PIN, GPIO_PIN_SET);
}

static inline void ST7735_DC_Command(void) {
    HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_RESET);
}

static inline void ST7735_DC_Data(void) {
    HAL_GPIO_WritePin(ST7735_DC_PORT, ST7735_DC_PIN, GPIO_PIN_SET);
}

static inline void ST7735_Reset(void) {
    HAL_GPIO_WritePin(ST7735_RST_PORT, ST7735_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(ST7735_RST_PORT, ST7735_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(120);
}

static void ST7735_WriteCommand(ST7735_Handle_t *hst7735, uint8_t cmd) {
    ST7735_DC_Command();
    HAL_SPI_Transmit(hst7735->hspi, &cmd, 1, HAL_MAX_DELAY);
}

static void ST7735_WriteData(ST7735_Handle_t *hst7735, uint8_t *data, uint16_t len) {
    ST7735_DC_Data();
    HAL_SPI_Transmit(hst7735->hspi, data, len, HAL_MAX_DELAY);
}

static void ST7735_WriteData8(ST7735_Handle_t *hst7735, uint8_t data) {
    ST7735_DC_Data();
    HAL_SPI_Transmit(hst7735->hspi, &data, 1, HAL_MAX_DELAY);
}

static void ST7735_SetAddressWindow(ST7735_Handle_t *hst7735,
                                    uint16_t x0, uint16_t y0,
                                    uint16_t x1, uint16_t y1) {
    ST7735_WriteCommand(hst7735, ST7735_CASET);
    uint8_t data[] = {
        0x00, (uint8_t)(x0 + ST7735_XSTART),
        0x00, (uint8_t)(x1 + ST7735_XSTART)
    };
    ST7735_WriteData(hst7735, data, 4);

    ST7735_WriteCommand(hst7735, ST7735_RASET);
    data[1] = y0 + ST7735_YSTART;
    data[3] = y1 + ST7735_YSTART;
    ST7735_WriteData(hst7735, data, 4);

    ST7735_WriteCommand(hst7735, ST7735_RAMWR);
}


void ST7735_Init(ST7735_Handle_t *hst7735, SPI_HandleTypeDef *hspi) {
    hst7735->hspi = hspi;
    hst7735->width = ST7735_WIDTH;
    hst7735->height = ST7735_HEIGHT;

    ST7735_Unselect();
    ST7735_Reset();
    ST7735_Select();

    ST7735_WriteCommand(hst7735, ST7735_SWRESET);
    HAL_Delay(150);

    ST7735_WriteCommand(hst7735, ST7735_SLPOUT);
    HAL_Delay(500);

    ST7735_WriteCommand(hst7735, ST7735_FRMCTR1);
    uint8_t frmctr[] = {0x01, 0x2C, 0x2D};
    ST7735_WriteData(hst7735, frmctr, 3);

    ST7735_WriteCommand(hst7735, ST7735_FRMCTR2);
    ST7735_WriteData(hst7735, frmctr, 3);

    ST7735_WriteCommand(hst7735, ST7735_FRMCTR3);
    ST7735_WriteData(hst7735, frmctr, 3);
    uint8_t frmctr2[] = {0x01, 0x2C, 0x2D};
    ST7735_WriteData(hst7735, frmctr2, 3);

    ST7735_WriteCommand(hst7735, ST7735_INVCTR);
    ST7735_WriteData8(hst7735, 0x07);

    ST7735_WriteCommand(hst7735, ST7735_PWCTR1);
    uint8_t pwctr1[] = {0xA2, 0x02, 0x84};
    ST7735_WriteData(hst7735, pwctr1, 3);

    ST7735_WriteCommand(hst7735, ST7735_PWCTR2);
    ST7735_WriteData8(hst7735, 0xC5);

    ST7735_WriteCommand(hst7735, ST7735_PWCTR3);
    uint8_t pwctr3[] = {0x0A, 0x00};
    ST7735_WriteData(hst7735, pwctr3, 2);

    ST7735_WriteCommand(hst7735, ST7735_PWCTR4);
    uint8_t pwctr4[] = {0x8A, 0x2A};
    ST7735_WriteData(hst7735, pwctr4, 2);

    ST7735_WriteCommand(hst7735, ST7735_PWCTR5);
    uint8_t pwctr5[] = {0x8A, 0xEE};
    ST7735_WriteData(hst7735, pwctr5, 2);

    ST7735_WriteCommand(hst7735, ST7735_VMCTR1);
    ST7735_WriteData8(hst7735, 0x0E);

    ST7735_WriteCommand(hst7735, ST7735_INVOFF);

    ST7735_WriteCommand(hst7735, ST7735_MADCTL);
    ST7735_WriteData8(hst7735, 0x00);  // Normal orientation

    ST7735_WriteCommand(hst7735, ST7735_COLMOD);
    ST7735_WriteData8(hst7735, 0x05);

    ST7735_WriteCommand(hst7735, ST7735_GMCTRP1);
    uint8_t gamma_p[] = {0x02, 0x1C, 0x07, 0x12, 0x37, 0x32, 0x29, 0x2D,
                         0x29, 0x25, 0x2B, 0x39, 0x00, 0x01, 0x03, 0x10};
    ST7735_WriteData(hst7735, gamma_p, 16);

    ST7735_WriteCommand(hst7735, ST7735_GMCTRN1);
    uint8_t gamma_n[] = {0x03, 0x1D, 0x07, 0x06, 0x2E, 0x2C, 0x29, 0x2D,
                         0x2E, 0x2E, 0x37, 0x3F, 0x00, 0x00, 0x02, 0x10};
    ST7735_WriteData(hst7735, gamma_n, 16);

    ST7735_WriteCommand(hst7735, ST7735_NORON);
    HAL_Delay(10);

    ST7735_WriteCommand(hst7735, ST7735_DISPON);
    HAL_Delay(100);

    ST7735_Unselect();

    ST7735_FillScreen(hst7735, ST7735_BLACK);
}

void ST7735_FillScreen(ST7735_Handle_t *hst7735, uint16_t color) {
    ST7735_FillRect(hst7735, 0, 0, hst7735->width, hst7735->height, color);
}

void ST7735_DrawPixel(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y, uint16_t color) {
    if (x >= hst7735->width || y >= hst7735->height) return;

    ST7735_Select();
    ST7735_SetAddressWindow(hst7735, x, y, x, y);
    uint8_t data[] = {color >> 8, color & 0xFF};
    ST7735_WriteData(hst7735, data, 2);
    ST7735_Unselect();
}

void ST7735_FillRect(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h, uint16_t color) {
    if (x >= hst7735->width || y >= hst7735->height) return;
    if (x + w > hst7735->width) w = hst7735->width - x;
    if (y + h > hst7735->height) h = hst7735->height - y;

    ST7735_Select();
    ST7735_SetAddressWindow(hst7735, x, y, x + w - 1, y + h - 1);

    uint8_t data[] = {color >> 8, color & 0xFF};
    ST7735_DC_Data();

    for (uint32_t i = 0; i < (uint32_t)w * h; i++) {
        HAL_SPI_Transmit(hst7735->hspi, data, 2, HAL_MAX_DELAY);
    }

    ST7735_Unselect();
}

void ST7735_DrawRect(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                     uint16_t w, uint16_t h, uint16_t color) {
    ST7735_DrawHLine(hst7735, x, y, w, color);
    ST7735_DrawHLine(hst7735, x, y + h - 1, w, color);
    ST7735_DrawVLine(hst7735, x, y, h, color);
    ST7735_DrawVLine(hst7735, x + w - 1, y, h, color);
}

void ST7735_DrawHLine(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      uint16_t w, uint16_t color) {
    ST7735_FillRect(hst7735, x, y, w, 1, color);
}

void ST7735_DrawVLine(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      uint16_t h, uint16_t color) {
    ST7735_FillRect(hst7735, x, y, 1, h, color);
}

void ST7735_DrawChar(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                     char c, uint16_t color, uint16_t bg, uint8_t size) {
    if (c < 32 || c > 127) c = '?';

    for (uint8_t i = 0; i < 5; i++) {
        uint8_t line = font5x7[(c - 32) * 5 + i];
        for (uint8_t j = 0; j < 8; j++) {
            if (line & (1 << j)) {
                if (size == 1) {
                    ST7735_DrawPixel(hst7735, x + i, y + j, color);
                } else {
                    ST7735_FillRect(hst7735, x + i * size, y + j * size, size, size, color);
                }
            } else if (bg != color) {
                if (size == 1) {
                    ST7735_DrawPixel(hst7735, x + i, y + j, bg);
                } else {
                    ST7735_FillRect(hst7735, x + i * size, y + j * size, size, size, bg);
                }
            }
        }
    }

    if (bg != color) {
        if (size == 1) {
            ST7735_DrawVLine(hst7735, x + 5, y, 8, bg);
        } else {
            ST7735_FillRect(hst7735, x + 5 * size, y, size, 8 * size, bg);
        }
    }
}

void ST7735_DrawString(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                       const char *str, uint16_t color, uint16_t bg, uint8_t size) {
    while (*str) {
        ST7735_DrawChar(hst7735, x, y, *str++, color, bg, size);
        x += 6 * size;
    }
}

void ST7735_DrawInt(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                    int32_t num, uint16_t color, uint16_t bg, uint8_t size) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%ld", num);
    ST7735_DrawString(hst7735, x, y, buf, color, bg, size);
}

void ST7735_DrawFloat(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                      float num, uint8_t decimals, uint16_t color, uint16_t bg, uint8_t size) {
    char buf[16];
    char fmt[8];
    snprintf(fmt, sizeof(fmt), "%%.%df", decimals);
    snprintf(buf, sizeof(buf), fmt, num);
    ST7735_DrawString(hst7735, x, y, buf, color, bg, size);
}

uint16_t ST7735_RGB(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

uint16_t ST7735_TempColor(float temp, float min_temp, float max_temp) {
    float ratio = (temp - min_temp) / (max_temp - min_temp);
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;

    uint8_t r, g, b;

    if (ratio < 0.5f) {
        float t = ratio * 2;
        r = 0;
        g = (uint8_t)(255 * t);
        b = (uint8_t)(255 * (1 - t));
    } else {
        float t = (ratio - 0.5f) * 2;
        r = (uint8_t)(255 * t);
        g = (uint8_t)(255 * (1 - t));
        b = 0;
    }

    return ST7735_RGB(r, g, b);
}

void ST7735_DrawProgressBar(ST7735_Handle_t *hst7735, uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h, uint16_t value, uint16_t max_value,
                            uint16_t bar_color, uint16_t bg_color) {
    ST7735_DrawRect(hst7735, x, y, w, h, ST7735_WHITE);

    ST7735_FillRect(hst7735, x + 1, y + 1, w - 2, h - 2, bg_color);

    uint16_t fill_w = ((uint32_t)(w - 2) * value) / max_value;
    if (fill_w > 0) {
        ST7735_FillRect(hst7735, x + 1, y + 1, fill_w, h - 2, bar_color);
    }
}

void ST7735_ShowTemperature(ST7735_Handle_t *hst7735, float temp, float setpoint,
                            uint16_t heater_pwm, uint16_t fan_pwm) {
    char buf[32];

    ST7735_DrawString(hst7735, 10, 5, "TEMP CONTROL", ST7735_CYAN, ST7735_BLACK, 1);

    ST7735_DrawHLine(hst7735, 0, 18, ST7735_WIDTH, ST7735_GRAY);

    uint16_t temp_color = ST7735_TempColor(temp, 20, 50);
    snprintf(buf, sizeof(buf), "%.1f", temp);
    ST7735_DrawString(hst7735, 20, 30, buf, temp_color, ST7735_BLACK, 3);
    ST7735_DrawString(hst7735, 95, 30, "C", temp_color, ST7735_BLACK, 2);

    ST7735_DrawRect(hst7735, 89, 30, 4, 4, temp_color);

    ST7735_DrawString(hst7735, 10, 60, "SET:", ST7735_GRAY, ST7735_BLACK, 1);
    snprintf(buf, sizeof(buf), "%.1f C", setpoint);
    ST7735_DrawString(hst7735, 40, 60, buf, ST7735_WHITE, ST7735_BLACK, 1);

    float error = setpoint - temp;
    ST7735_DrawString(hst7735, 10, 75, "ERR:", ST7735_GRAY, ST7735_BLACK, 1);
    snprintf(buf, sizeof(buf), "%+.2f", error);
    uint16_t err_color = (error > 0) ? ST7735_BLUE : ST7735_RED;
    if (error > -0.5f && error < 0.5f) err_color = ST7735_GREEN;
    ST7735_DrawString(hst7735, 40, 75, buf, err_color, ST7735_BLACK, 1);

    ST7735_DrawHLine(hst7735, 0, 90, ST7735_WIDTH, ST7735_GRAY);

    ST7735_DrawString(hst7735, 5, 100, "HEAT:", ST7735_ORANGE, ST7735_BLACK, 1);
    ST7735_DrawProgressBar(hst7735, 40, 98, 80, 12, heater_pwm, 999, ST7735_RED, ST7735_DARKGRAY);

    ST7735_DrawString(hst7735, 5, 118, "FAN:", ST7735_CYAN, ST7735_BLACK, 1);
    ST7735_DrawProgressBar(hst7735, 40, 116, 80, 12, fan_pwm, 999, ST7735_BLUE, ST7735_DARKGRAY);

    snprintf(buf, sizeof(buf), "%3d", heater_pwm);
    ST7735_DrawString(hst7735, 5, 135, "H:", ST7735_GRAY, ST7735_BLACK, 1);
    ST7735_DrawString(hst7735, 20, 135, buf, ST7735_WHITE, ST7735_BLACK, 1);

    snprintf(buf, sizeof(buf), "%3d", fan_pwm);
    ST7735_DrawString(hst7735, 65, 135, "F:", ST7735_GRAY, ST7735_BLACK, 1);
    ST7735_DrawString(hst7735, 80, 135, buf, ST7735_WHITE, ST7735_BLACK, 1);
}
