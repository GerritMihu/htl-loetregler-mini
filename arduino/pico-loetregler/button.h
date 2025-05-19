#ifndef BUTTON_H
#define BUTTON_H

#define bool _Bool

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
} ButtonType;

void button_init();
void button_update();
bool button_pressed(ButtonType btn);

#endif
