
#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "button.h"

#define BUTTON_DEBOUNCE_DELAY 20 // Debounce delay in milliseconds
#define NUM_BUTTONS 4 // Number of buttons

// Arrays to store button states and events
static bool button_state[NUM_BUTTONS] = {false, false, false, false};
static bool button_last_state[NUM_BUTTONS] = {false, false, false, false};
static uint32_t button_last_change[NUM_BUTTONS] = {0, 0, 0, 0};
static bool button_is_pressed[NUM_BUTTONS] = {false, false, false, false};
static bool button_is_released[NUM_BUTTONS] = {false, false, false, false};

// Array to store button GPIO pins
static const uint8_t button_pins[NUM_BUTTONS] = {
    BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT
};

// Helper function to get the index of a button
static inline int get_button_index(ButtonType btn) {
    if (btn >= BUTTON_UP && btn <= BUTTON_LEFT) {
        return btn - BUTTON_UP;
    }
    return -1;
}

// Initialize the buttons
void button_init(void) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(button_pins[i]);
        gpio_set_dir(button_pins[i], GPIO_IN);
        gpio_pull_up(button_pins[i]);
    }
}

// Update the button states (should be called regularly)
void button_update(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool current_state = !gpio_get(button_pins[i]); // Active low (Pull-up)

        // Button state has changed
        if (current_state != button_last_state[i]) {
            button_last_change[i] = now;
            button_last_state[i] = current_state;
        }

        // Debounce logic
        if (now - button_last_change[i] > BUTTON_DEBOUNCE_DELAY) {
            // Detect pressed event
            if (current_state && !button_state[i]) {
                button_is_pressed[i] = true;
            } else {
                button_is_pressed[i] = false;
            }

            // Detect released event
            if (!current_state && button_state[i]) {
                button_is_released[i] = true;
            } else {
                button_is_released[i] = false;
            }

            button_state[i] = current_state;
        }
    }
}

// Check if a button was pressed (single event)
bool button_pressed(ButtonType btn) {
    int index = get_button_index(btn);
    if (index == -1) return false;

    bool result = button_is_pressed[index];
    button_is_pressed[index] = false; // Reset after reading
    return result;
}

// Check if a button was released (single event)
bool button_released(ButtonType btn) {
    int index = get_button_index(btn);
    if (index == -1) return false;

    bool result = button_is_released[index];
    button_is_released[index] = false; // Reset after reading
    return result;
}

// Check if a button is held down (continuous state)
bool button_held(ButtonType btn) {
    int index = get_button_index(btn);
    if (index == -1) return false;

    return button_state[index];
}

// Optional: Debug function to print button states
void button_debug_print(void) {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        printf("Button %d: state=%d, pressed=%d, released=%d\n",
               button_pins[i], button_state[i], button_is_pressed[i], button_is_released[i]);
    }
}
