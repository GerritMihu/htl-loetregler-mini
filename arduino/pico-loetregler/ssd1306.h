#ifndef SSD1306_H
#define SSD1306_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdint.h>
#include <string.h>

// SSD1306 Befehle
#define SSD1306_DISPLAYOFF          0xAE
#define SSD1306_DISPLAYON           0xAF
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5
#define SSD1306_SETMULTIPLEX        0xA8
#define SSD1306_SETDISPLAYOFFSET    0xD3
#define SSD1306_SETSTARTLINE        0x40
#define SSD1306_CHARGEPUMP          0x8D
#define SSD1306_MEMORYMODE          0x20
#define SSD1306_SEGREMAP            0xA0
#define SSD1306_COMSCANDEC          0xC8
#define SSD1306_SETCOMPINS          0xDA
#define SSD1306_SETCONTRAST         0x81
#define SSD1306_SETPRECHARGE        0xD9
#define SSD1306_SETVCOMDETECT       0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY       0xA6
#define SSD1306_INVERTDISPLAY       0xA7
#define SSD1306_SETLOWCOLUMN        0x00
#define SSD1306_SETHIGHCOLUMN       0x10
#define SSD1306_SETSTARTPAGE        0xB0
#define SSD1306_COLUMNADDR          0x21
#define SSD1306_PAGEADDR            0x22

// Strukturdefinition
typedef struct {
    i2c_inst_t *i2c;
    uint8_t address;
    uint8_t width;
    uint8_t height;
    uint8_t pages;
    uint8_t *buffer;
} ssd1306_t;

// Funktionsprototypen
void ssd1306_init(ssd1306_t *display, i2c_inst_t *i2c_instance, uint8_t address, uint8_t width, uint8_t height);
void ssd1306_command(ssd1306_t *display, uint8_t command);
void ssd1306_commands(ssd1306_t *display, uint8_t *commands, uint8_t count);
void ssd1306_clear(ssd1306_t *display);
void ssd1306_show(ssd1306_t *display);
void ssd1306_set_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool on);
void ssd1306_draw_char(ssd1306_t *display, uint8_t x, uint8_t y, uint8_t scale, char c);
void ssd1306_draw_string(ssd1306_t *display, uint8_t x, uint8_t y, uint8_t scale, const char *s);

#endif // SSD1306_H
