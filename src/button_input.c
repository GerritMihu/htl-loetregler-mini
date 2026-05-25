#include "button_input.h"

#include "hardware/gpio.h"

#define BUTTON_DEBOUNCE_TIME_MS 20U

#define PIN_TASTER_EINAUS 4U
#define PIN_TASTER_UP 5U
#define PIN_TASTER_DOWN 6U
#define PIN_TASTER_ENTER 7U
#define PIN_TASTER_BACK 8U

void button_init(button_t *button, uint8_t pin) {
  button->pin = pin;
  button->stable_pressed = false;
  button->last_raw_pressed = false;
  button->event = BUTTON_EVENT_NONE;
  button->last_change_ms = 0;

  gpio_init(pin);
  gpio_set_dir(pin, GPIO_IN);
  gpio_pull_up(pin);
}

void button_update(button_t *button, uint32_t now_ms) {
  bool raw_pressed;

  // Die Taster sind als Pull-up beschaltet.
  // LOW bedeutet also "gedrueckt".
  raw_pressed = !gpio_get(button->pin);
  if (raw_pressed != button->last_raw_pressed) {
    button->last_raw_pressed = raw_pressed;
    button->last_change_ms = now_ms;
  }

  // Erst wenn der Pegel lange genug stabil bleibt,
  // uebernehmen wir ihn als echten Tasterzustand.
  if ((now_ms - button->last_change_ms) < BUTTON_DEBOUNCE_TIME_MS) {
    return;
  }

  if (raw_pressed != button->stable_pressed) {
    button->stable_pressed = raw_pressed;
    button->event = raw_pressed ? BUTTON_EVENT_PRESSED : BUTTON_EVENT_RELEASED;
  }
}

button_event_t button_take_event(button_t *button) {
  button_event_t event;

  event = button->event;
  button->event = BUTTON_EVENT_NONE;
  return event;
}

bool button_is_pressed(const button_t *button) {
  return button->stable_pressed;
}

void button_panel_begin(button_panel_t *panel) {
  button_init(&panel->power, PIN_TASTER_EINAUS);
  button_init(&panel->up, PIN_TASTER_UP);
  button_init(&panel->down, PIN_TASTER_DOWN);
  button_init(&panel->enter, PIN_TASTER_ENTER);
  button_init(&panel->back, PIN_TASTER_BACK);
}

void button_panel_update(button_panel_t *panel, uint32_t now_ms) {
  button_update(&panel->power, now_ms);
  button_update(&panel->up, now_ms);
  button_update(&panel->down, now_ms);
  button_update(&panel->enter, now_ms);
  button_update(&panel->back, now_ms);
}
