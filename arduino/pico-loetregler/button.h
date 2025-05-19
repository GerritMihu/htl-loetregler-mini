
#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

// Define button types using an enum for clarity
typedef enum {
    BUTTON_ONOFF = 4, // Button ON/OFF
    BUTTON_UP,        // Button UP
    BUTTON_DOWN,      // Button DOWN
    BUTTON_RIGHT,     // Button RIGHT
    BUTTON_LEFT       // Button LEFT
} ButtonType;

// Function to initialize the buttons
void button_init(void);

// Function to update the button states (should be called regularly)
void button_update(void);

// Function to check if a button was pressed (single event)
bool button_pressed(ButtonType btn);

// Function to check if a button was released (single event)
bool button_released(ButtonType btn);

// Function to check if a button is held down (continuous state)
bool button_held(ButtonType btn);

#endif
