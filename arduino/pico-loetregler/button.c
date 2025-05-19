#include <stdlib.h>
#include <stdio.h>

#include "pico/stdlib.h"
#include "button.h"

#define BUTTON_DEBOUNCE_DELAY 50 // Entprellzeit in Millisekunden

static bool button_state[4] = {false, false, false, false};
static bool button_last_state[4] = {false, false, false, false};
static uint32_t button_last_change[4] = {0, 0, 0, 0};
static bool button_is_pressed[4] = {false, false, false, false};
static bool button_is_released[4] = {false, false, false, false};

// Initialisiere die Buttons
void button_init(void) {
    gpio_init(BUTTON_UP);
    gpio_init(BUTTON_DOWN);
    gpio_init(BUTTON_LEFT);
    gpio_init(BUTTON_RIGHT);

    gpio_set_dir(BUTTON_UP, GPIO_IN);
    gpio_set_dir(BUTTON_DOWN, GPIO_IN);
    gpio_set_dir(BUTTON_LEFT, GPIO_IN);
    gpio_set_dir(BUTTON_RIGHT, GPIO_IN);

    gpio_pull_up(BUTTON_UP);
    gpio_pull_up(BUTTON_DOWN);
    gpio_pull_up(BUTTON_LEFT);
    gpio_pull_up(BUTTON_RIGHT);
}

// Aktualisiere die Button-Zustände (muss regelmäßig aufgerufen werden)
void button_update(void) {
    uint8_t buttons[4] = {BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT};
    uint32_t now = to_ms_since_boot(get_absolute_time());

    for (int i = 0; i < 4; i++) {
        bool current_state = !gpio_get(buttons[i]); // Active low (Pull-up)

        // Button-Zustand hat sich geändert
        if (current_state != button_last_state[i]) {
            button_last_change[i] = now;
            button_last_state[i] = current_state;
        }

        // Entprellen
        if (now - button_last_change[i] > BUTTON_DEBOUNCE_DELAY) {
            // Erkennt pressed Event
            if (current_state && !button_state[i]) {
                button_is_pressed[i] = true;
            } else {
                button_is_pressed[i] = false;
            }

            // Erkennt released Event
            if (!current_state && button_state[i]) {
                button_is_released[i] = true;
            } else {
                button_is_released[i] = false;
            }

            button_state[i] = current_state;
        }
    }
}

// Prüfe, ob ein Button gedrückt wurde (einmaliger Event)
bool button_pressed(uint8_t button) {
    uint8_t index;

    if (button == BUTTON_UP) index = 0;
    else if (button == BUTTON_DOWN) index = 1;
    else if (button == BUTTON_LEFT) index = 2;
    else if (button == BUTTON_RIGHT) index = 3;
    else return false;

    bool result = button_is_pressed[index];
    button_is_pressed[index] = false; // Reset nach dem Lesen
    return result;
}

// Prüfe, ob ein Button losgelassen wurde (einmaliger Event)
bool button_released(uint8_t button) {
    uint8_t index;

    if (button == BUTTON_UP) index = 0;
    else if (button == BUTTON_DOWN) index = 1;
    else if (button == BUTTON_LEFT) index = 2;
    else if (button == BUTTON_RIGHT) index = 3;
    else return false;

    bool result = button_is_released[index];
    button_is_released[index] = false; // Reset nach dem Lesen
    return result;
}

// Prüfe, ob ein Button gehalten wird (kontinuierlicher Zustand)
bool button_held(uint8_t button) {
    if (button == BUTTON_UP) return button_state[0];
    else if (button == BUTTON_DOWN) return button_state[1];
    else if (button == BUTTON_LEFT) return button_state[2];
    else if (button == BUTTON_RIGHT) return button_state[3];
    else return false;
}
