#include "LoetreglerLogic.h"

#include <stdio.h>

static const char *const GREETINGS[LOETREGLER_GREETING_COUNT] = {
    "HTL STEYR",
    "WILLKOMMEN",
    "LOETREGLER",
    "VIEL ERFOLG",
};

static const menu_item_t MENU_ITEMS[LOETREGLER_MENU_ITEM_COUNT] = {
    MENU_ITEM_STARTUP_TEMPERATURE,
    MENU_ITEM_GREETING,
    MENU_ITEM_MAX_TEMPERATURE,
    MENU_ITEM_SHUTDOWN_MINUTES,
    MENU_ITEM_STANDBY_TEMPERATURE,
    MENU_ITEM_TEMPERATURE_STEP,
    MENU_ITEM_FACTORY_RESET,
    MENU_ITEM_EXIT,
};

static uint16_t clamp_u16(uint16_t value, uint16_t minimum, uint16_t maximum) {
  if (value < minimum) {
    return minimum;
  }
  if (value > maximum) {
    return maximum;
  }
  return value;
}

loetregler_settings_t loetregler_default_settings(void) {
  loetregler_settings_t settings;

  settings.startup_temperature_c = 330;
  settings.max_temperature_c = 380;
  settings.standby_temperature_c = 150;
  settings.shutdown_minutes = 10;
  settings.temperature_step_c = 10;
  settings.greeting_index = 0;
  loetregler_sanitize_settings(&settings);
  return settings;
}

void loetregler_sanitize_settings(loetregler_settings_t *settings) {
  settings->max_temperature_c = clamp_u16(settings->max_temperature_c, 200, LOETREGLER_HARD_MAX_TEMPERATURE_C);
  settings->startup_temperature_c = clamp_u16(settings->startup_temperature_c, LOETREGLER_MIN_TEMPERATURE_C, settings->max_temperature_c);
  settings->standby_temperature_c = clamp_u16(settings->standby_temperature_c, LOETREGLER_MIN_TEMPERATURE_C, settings->max_temperature_c);
  settings->shutdown_minutes = clamp_u16(settings->shutdown_minutes, 1, 120);
  settings->temperature_step_c = clamp_u16(settings->temperature_step_c, 5, 50);

  if (settings->greeting_index >= LOETREGLER_GREETING_COUNT) {
    settings->greeting_index = 0;
  }
}

uint16_t loetregler_clamp_setpoint(uint16_t requested_temperature_c, const loetregler_settings_t *settings) {
  return clamp_u16(requested_temperature_c, LOETREGLER_MIN_TEMPERATURE_C, settings->max_temperature_c);
}

uint16_t loetregler_active_target_temperature(uint16_t manual_setpoint_c, bool standby, const loetregler_settings_t *settings) {
  if (standby) {
    return settings->standby_temperature_c;
  }
  return loetregler_clamp_setpoint(manual_setpoint_c, settings);
}

uint32_t loetregler_shutdown_timeout_ms(const loetregler_settings_t *settings) {
  return (uint32_t) settings->shutdown_minutes * 60UL * 1000UL;
}

const char *loetregler_greeting_text(uint8_t greeting_index) {
  if (greeting_index >= LOETREGLER_GREETING_COUNT) {
    return GREETINGS[0];
  }
  return GREETINGS[greeting_index];
}

menu_item_t loetregler_current_menu_item(const menu_state_t *menu_state) {
  return MENU_ITEMS[menu_state->selected_item % LOETREGLER_MENU_ITEM_COUNT];
}

const char *loetregler_menu_label(menu_item_t item) {
  switch (item) {
    case MENU_ITEM_STARTUP_TEMPERATURE:
      return "Starttemp";
    case MENU_ITEM_GREETING:
      return "Gruss";
    case MENU_ITEM_MAX_TEMPERATURE:
      return "Max Temp";
    case MENU_ITEM_SHUTDOWN_MINUTES:
      return "Auto Aus";
    case MENU_ITEM_STANDBY_TEMPERATURE:
      return "Standby";
    case MENU_ITEM_TEMPERATURE_STEP:
      return "Schritt";
    case MENU_ITEM_FACTORY_RESET:
      return "Reset";
    case MENU_ITEM_EXIT:
      return "Zurueck";
  }
  return "?";
}

void loetregler_move_menu_selection(menu_state_t *menu_state, int direction) {
  int next_index;

  if (menu_state->editing_value) {
    return;
  }

  next_index = (int) menu_state->selected_item + direction;
  if (next_index < 0) {
    next_index = LOETREGLER_MENU_ITEM_COUNT - 1;
  } else if (next_index >= LOETREGLER_MENU_ITEM_COUNT) {
    next_index = 0;
  }
  menu_state->selected_item = (uint8_t) next_index;
}

bool loetregler_change_menu_value(loetregler_settings_t *settings, const menu_state_t *menu_state, int direction) {
  int next_value;

  if (!menu_state->editing_value) {
    return false;
  }

  switch (loetregler_current_menu_item(menu_state)) {
    case MENU_ITEM_STARTUP_TEMPERATURE:
      next_value = (int) settings->startup_temperature_c + direction * 10;
      settings->startup_temperature_c = next_value < 0 ? 0 : (uint16_t) next_value;
      break;
    case MENU_ITEM_GREETING:
      next_value = (int) settings->greeting_index + direction;
      if (next_value < 0) {
        next_value = LOETREGLER_GREETING_COUNT - 1;
      } else if (next_value >= LOETREGLER_GREETING_COUNT) {
        next_value = 0;
      }
      settings->greeting_index = (uint8_t) next_value;
      break;
    case MENU_ITEM_MAX_TEMPERATURE:
      next_value = (int) settings->max_temperature_c + direction * 10;
      settings->max_temperature_c = next_value < 0 ? 0 : (uint16_t) next_value;
      break;
    case MENU_ITEM_SHUTDOWN_MINUTES:
      next_value = (int) settings->shutdown_minutes + direction;
      settings->shutdown_minutes = next_value < 0 ? 0 : (uint16_t) next_value;
      break;
    case MENU_ITEM_STANDBY_TEMPERATURE:
      next_value = (int) settings->standby_temperature_c + direction * 10;
      settings->standby_temperature_c = next_value < 0 ? 0 : (uint16_t) next_value;
      break;
    case MENU_ITEM_TEMPERATURE_STEP:
      next_value = (int) settings->temperature_step_c + direction * 5;
      settings->temperature_step_c = next_value < 0 ? 0 : (uint16_t) next_value;
      break;
    case MENU_ITEM_FACTORY_RESET:
    case MENU_ITEM_EXIT:
      return false;
  }

  loetregler_sanitize_settings(settings);
  return true;
}

menu_enter_result_t loetregler_activate_menu_item(menu_state_t *menu_state, loetregler_settings_t *settings) {
  if (menu_state->editing_value) {
    menu_state->editing_value = false;
    return MENU_ENTER_EDIT_MODE_CHANGED;
  }

  switch (loetregler_current_menu_item(menu_state)) {
    case MENU_ITEM_FACTORY_RESET:
      *settings = loetregler_default_settings();
      return MENU_ENTER_SETTINGS_CHANGED;
    case MENU_ITEM_EXIT:
      return MENU_ENTER_CLOSE_MENU;
    default:
      menu_state->editing_value = true;
      return MENU_ENTER_EDIT_MODE_CHANGED;
  }
}

bool loetregler_menu_back_leaves_menu(menu_state_t *menu_state) {
  if (menu_state->editing_value) {
    menu_state->editing_value = false;
    return false;
  }
  return true;
}

void loetregler_menu_value_text(const loetregler_settings_t *settings, menu_item_t item, char *buffer, size_t buffer_size) {
  if (buffer_size == 0U) {
    return;
  }

  switch (item) {
    case MENU_ITEM_STARTUP_TEMPERATURE:
      snprintf(buffer, buffer_size, "%uC", settings->startup_temperature_c);
      break;
    case MENU_ITEM_GREETING:
      snprintf(buffer, buffer_size, "%s", loetregler_greeting_text(settings->greeting_index));
      break;
    case MENU_ITEM_MAX_TEMPERATURE:
      snprintf(buffer, buffer_size, "%uC", settings->max_temperature_c);
      break;
    case MENU_ITEM_SHUTDOWN_MINUTES:
      snprintf(buffer, buffer_size, "%umin", settings->shutdown_minutes);
      break;
    case MENU_ITEM_STANDBY_TEMPERATURE:
      snprintf(buffer, buffer_size, "%uC", settings->standby_temperature_c);
      break;
    case MENU_ITEM_TEMPERATURE_STEP:
      snprintf(buffer, buffer_size, "%uC", settings->temperature_step_c);
      break;
    case MENU_ITEM_FACTORY_RESET:
    case MENU_ITEM_EXIT:
      snprintf(buffer, buffer_size, "ENTER");
      break;
  }
}
