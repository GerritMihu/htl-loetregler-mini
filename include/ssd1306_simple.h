#ifndef SSD1306_SIMPLE_H
#define SSD1306_SIMPLE_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

#define SSD1306_MAX_BUFFER_SIZE 512

typedef struct {
  i2c_inst_t *i2c;
  uint8_t address;
  uint8_t width;
  uint8_t height;
  uint8_t pages;
  uint8_t buffer[SSD1306_MAX_BUFFER_SIZE];
} ssd1306_t;

void ssd1306_init(ssd1306_t *display, i2c_inst_t *i2c_instance, uint8_t address, uint8_t width, uint8_t height);
void ssd1306_clear(ssd1306_t *display);
void ssd1306_show(ssd1306_t *display);
void ssd1306_set_pixel(ssd1306_t *display, uint8_t x, uint8_t y, bool on);
void ssd1306_draw_char(ssd1306_t *display, uint8_t x, uint8_t y, uint8_t scale, char character);
void ssd1306_draw_string(ssd1306_t *display, uint8_t x, uint8_t y, uint8_t scale, const char *text);

#endif
