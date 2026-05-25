#ifndef BUTTON_INPUT_H
#define BUTTON_INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  BUTTON_EVENT_NONE = 0,
  BUTTON_EVENT_PRESSED,
  BUTTON_EVENT_RELEASED
} button_event_t;

typedef struct {
  uint8_t pin;
  bool stable_pressed;
  bool last_raw_pressed;
  button_event_t event;
  uint32_t last_change_ms;
} button_t;

typedef struct {
  button_t power;
  button_t up;
  button_t down;
  button_t enter;
  button_t back;
} button_panel_t;

void button_init(button_t *button, uint8_t pin);
void button_update(button_t *button, uint32_t now_ms);
button_event_t button_take_event(button_t *button);
bool button_is_pressed(const button_t *button);

void button_panel_begin(button_panel_t *panel);
void button_panel_update(button_panel_t *panel, uint32_t now_ms);

#endif
