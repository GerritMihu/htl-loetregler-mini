#ifndef LOETREGLER_LOGIC_H
#define LOETREGLER_LOGIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LOETREGLER_MIN_TEMPERATURE_C 50
#define LOETREGLER_HARD_MAX_TEMPERATURE_C 450
#define LOETREGLER_GREETING_COUNT 4
#define LOETREGLER_MENU_ITEM_COUNT 8
#define LOETREGLER_SETTINGS_MAGIC 0x4C4F4554UL
#define LOETREGLER_SETTINGS_VERSION 1

typedef struct {
  uint16_t startup_temperature_c;
  uint16_t max_temperature_c;
  uint16_t standby_temperature_c;
  uint16_t shutdown_minutes;
  uint16_t temperature_step_c;
  uint8_t greeting_index;
} loetregler_settings_t;

typedef enum {
  MENU_ITEM_STARTUP_TEMPERATURE = 0,
  MENU_ITEM_GREETING,
  MENU_ITEM_MAX_TEMPERATURE,
  MENU_ITEM_SHUTDOWN_MINUTES,
  MENU_ITEM_STANDBY_TEMPERATURE,
  MENU_ITEM_TEMPERATURE_STEP,
  MENU_ITEM_FACTORY_RESET,
  MENU_ITEM_EXIT
} menu_item_t;

typedef enum {
  MENU_ENTER_NOTHING = 0,
  MENU_ENTER_EDIT_MODE_CHANGED,
  MENU_ENTER_SETTINGS_CHANGED,
  MENU_ENTER_CLOSE_MENU
} menu_enter_result_t;

typedef struct {
  uint8_t selected_item;
  bool editing_value;
} menu_state_t;

loetregler_settings_t loetregler_default_settings(void);
void loetregler_sanitize_settings(loetregler_settings_t *settings);
uint16_t loetregler_clamp_setpoint(uint16_t requested_temperature_c, const loetregler_settings_t *settings);
uint16_t loetregler_active_target_temperature(uint16_t manual_setpoint_c, bool standby, const loetregler_settings_t *settings);
uint32_t loetregler_shutdown_timeout_ms(const loetregler_settings_t *settings);
const char *loetregler_greeting_text(uint8_t greeting_index);
menu_item_t loetregler_current_menu_item(const menu_state_t *menu_state);
const char *loetregler_menu_label(menu_item_t item);
void loetregler_move_menu_selection(menu_state_t *menu_state, int direction);
bool loetregler_change_menu_value(loetregler_settings_t *settings, const menu_state_t *menu_state, int direction);
menu_enter_result_t loetregler_activate_menu_item(menu_state_t *menu_state, loetregler_settings_t *settings);
bool loetregler_menu_back_leaves_menu(menu_state_t *menu_state);
void loetregler_menu_value_text(const loetregler_settings_t *settings, menu_item_t item, char *buffer, size_t buffer_size);

#endif
