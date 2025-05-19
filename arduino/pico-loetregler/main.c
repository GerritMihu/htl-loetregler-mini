// main.c
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "button.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define I2C_PORT i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

#define PIN_SELBSHALTUNG 2
#define PIN_HEIZELEMENT 3
#define PIN_COMTREIBER_ENABLE 9

#define ADC_UBATT 0  // GPIO26
#define ADC_TEMPSENSOR 1 // GPIO27
#define ADC_STROM 2 // GPIO28

#define DIVISOR 2.0f
#define SPANNUNG_VOLL (40.0f / DIVISOR)
#define SPANNUNG_LEER (32.0f / DIVISOR)

static ssd1306_t display;

uint16_t tempSoll = 330;
uint16_t tempSpitze = 999;
bool standby = false;
bool forcedShutdown = false;
uint32_t timeLastTempIncrease = 0;
float power = 0;
float uBatt = 0;

void selbsthaltung() {
    gpio_put(PIN_SELBSHALTUNG, 1);
}

float read_adc_voltage(uint adc_channel) {
    adc_select_input(adc_channel);
    uint16_t raw = adc_read();
    return raw * 3.0f / 4095.0f;
}

void init_display() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    ssd1306_init(&display, I2C_PORT, 0x3C, SCREEN_WIDTH, SCREEN_HEIGHT);
    ssd1306_clear(&display);
    ssd1306_draw_string(&display, 0, 0, 1, "HTL LOETREGLER MINI");
    ssd1306_show(&display);
}

int main() {
    stdio_init_all();
    gpio_init(PIN_SELBSHALTUNG);
    gpio_set_dir(PIN_SELBSHALTUNG, GPIO_OUT);
    gpio_init(PIN_HEIZELEMENT);
    gpio_set_dir(PIN_HEIZELEMENT, GPIO_OUT);
    gpio_init(PIN_COMTREIBER_ENABLE);
    gpio_set_dir(PIN_COMTREIBER_ENABLE, GPIO_OUT);

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);

    button_init();
    init_display();
    selbsthaltung();

    while (true) {
        button_update();

        if (button_pressed(BUTTON_UP)) {
            tempSoll += 10;
            timeLastTempIncrease = to_ms_since_boot(get_absolute_time());
        }

        if (button_pressed(BUTTON_DOWN)) {
            tempSoll -= 10;
        }

        float temp = read_adc_voltage(ADC_TEMPSENSOR) * 100.0f;
        uBatt = read_adc_voltage(ADC_UBATT) * (40.0f / 3.0f);

        bool heizen = temp < tempSoll && !standby;
        gpio_put(PIN_HEIZELEMENT, heizen);

        ssd1306_clear(&display);
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Temp: %.1f / %d", temp, tempSoll);
        ssd1306_draw_string(&display, 0, 0, 1, buffer);
        snprintf(buffer, sizeof(buffer), "U: %.2fV", uBatt);
        ssd1306_draw_string(&display, 0, 10, 1, buffer);
        ssd1306_show(&display);

        sleep_ms(200);
    }
    return 0;
}

