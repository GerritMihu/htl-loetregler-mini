#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"

#include <LoetreglerLogic.h>
#include "button_input.h"
#include "settings_store.h"
#include "ssd1306_simple.h"

#define SCREEN_WIDTH 128U
#define SCREEN_HEIGHT 32U

#define I2C_PORT i2c0
#define PIN_I2C_SDA 20U
#define PIN_I2C_SCL 21U

#define PIN_POWER_HOLD 2U
#define PIN_HEATER 3U
#define PIN_COM_DRIVER_ENABLE 9U

#define ADC_CHANNEL_BATTERY 0U
#define ADC_CHANNEL_TIP_SENSOR 1U
#define ADC_CHANNEL_CURRENT 2U

#define GPIO_ADC_BATTERY 26U
#define GPIO_ADC_TIP_SENSOR 27U
#define GPIO_ADC_CURRENT 28U

#define ADC_REFERENCE_VOLTAGE 3.0f
#define BATTERY_DIVIDER_FACTOR 23.0f
#define CURRENT_SHUNT_OHM 0.001f
#define AMPLIFIER_GAIN 221.0f
#define TIP_SENSOR_VOLT_PER_KELVIN 24e-6f

#define BATTERY_EMPTY_VOLTAGE 16.0f
#define TIP_SENSOR_ERROR_TEMPERATURE_C 490U
#define SAFE_SWITCH_OFF_TEMPERATURE_C 60U

#define PWM_WRAP_VALUE 1249U
#define PWM_CLOCK_DIVIDER 5.0f
#define PWM_HEATING_FAST 250U
#define PWM_HEATING_SOFT 220U

typedef enum {
  SCREEN_MAIN = 0,
  SCREEN_MENU,
  SCREEN_COOLING
} screen_state_t;

typedef struct {
  uint16_t tip_temperature_c;
  uint16_t manual_setpoint_c;
  uint16_t active_target_c;
  float battery_voltage_v;
  float current_ampere;
  float power_watt;
  bool standby;
  bool forced_shutdown;
  uint32_t last_user_action_ms;
  screen_state_t screen;
} runtime_state_t;

static ssd1306_t display;
static button_panel_t buttons;
static loetregler_settings_t settings;
static menu_state_t menu_state;
static runtime_state_t runtime_state;
static uint pwm_slice;

static uint32_t now_ms(void) {
  return to_ms_since_boot(get_absolute_time());
}

static void note_user_activity(void) {
  runtime_state.last_user_action_ms = now_ms();
}

static float read_adc_voltage(uint channel) {
  uint16_t raw_value;

  // Der ADC liefert einen rohen Ganzzahlwert.
  // Fuer die restliche Logik rechnen wir ihn sofort in Volt um,
  // damit alle spaeteren Formeln leichter lesbar bleiben.
  adc_select_input(channel);
  raw_value = adc_read();
  return (float) raw_value * ADC_REFERENCE_VOLTAGE / 4095.0f;
}

static float read_battery_voltage(void) {
  return read_adc_voltage(ADC_CHANNEL_BATTERY) * BATTERY_DIVIDER_FACTOR;
}

static uint16_t read_tip_temperature_c(void) {
  float sensor_voltage;
  float temperature;

  // Die Lötspitze wird nicht direkt gemessen.
  // Wir messen zuerst die verstaerkte Sensorspannung
  // und rechnen sie dann ueber die bekannte Kennlinie in Grad um.
  sensor_voltage = read_adc_voltage(ADC_CHANNEL_TIP_SENSOR);
  temperature = sensor_voltage / AMPLIFIER_GAIN / TIP_SENSOR_VOLT_PER_KELVIN + 30.0f;
  return (uint16_t) temperature;
}

static float read_heater_current_a(void) {
  float shunt_voltage;

  shunt_voltage = read_adc_voltage(ADC_CHANNEL_CURRENT);
  return shunt_voltage / AMPLIFIER_GAIN / CURRENT_SHUNT_OHM;
}

static void sync_setpoint_with_settings(void) {
  runtime_state.manual_setpoint_c = loetregler_clamp_setpoint(runtime_state.manual_setpoint_c, &settings);
}

static void draw_text_screen(const char *line1, const char *line2, const char *line3, const char *line4) {
  ssd1306_clear(&display);
  ssd1306_draw_string(&display, 0, 0, 1, line1);
  ssd1306_draw_string(&display, 0, 8, 1, line2);
  ssd1306_draw_string(&display, 0, 16, 1, line3);
  ssd1306_draw_string(&display, 0, 24, 1, line4);
  ssd1306_show(&display);
}

static void show_greeting_screen(void) {
  draw_text_screen("HTL LOETREGLER",
                   loetregler_greeting_text(settings.greeting_index),
                   "PICO SDK / C",
                   "Start...");
  sleep_ms(1400);
}

static void draw_main_screen(void) {
  char line1[32];
  char line2[32];
  char line3[32];

  snprintf(line1, sizeof(line1), "IST %3uC SOLL %3uC", runtime_state.tip_temperature_c, runtime_state.active_target_c);
  snprintf(line2, sizeof(line2), "U %4.1fV  P %3uW", runtime_state.battery_voltage_v, (unsigned) runtime_state.power_watt);
  snprintf(line3, sizeof(line3), "%s  Schritt %uC", runtime_state.standby ? "STANDBY" : "AKTIV", settings.temperature_step_c);

  draw_text_screen(line1, line2, line3, "ENT Menue  BACK Std");
}

static void draw_menu_screen(void) {
  uint8_t first_visible_item;
  uint8_t row;

  // Das Menue zeigt immer nur drei Eintraege gleichzeitig.
  // So bleibt die Navigation fuer Schueler uebersichtlich
  // und passt gut auf das kleine 128x32-Display.
  ssd1306_clear(&display);
  ssd1306_draw_string(&display, 0, 0, 1, menu_state.editing_value ? "MENUE: WERT AENDERN" : "MENUE");

  first_visible_item = 0;
  if (menu_state.selected_item >= 2U) {
    first_visible_item = (uint8_t) (menu_state.selected_item - 2U);
  }
  if (first_visible_item > (uint8_t) (LOETREGLER_MENU_ITEM_COUNT - 3U)) {
    first_visible_item = LOETREGLER_MENU_ITEM_COUNT - 3U;
  }

  for (row = 0; row < 3U; ++row) {
    uint8_t item_index;
    menu_state_t item_state;
    menu_item_t item;
    char value_text[21];
    char line_buffer[40];

    item_index = (uint8_t) (first_visible_item + row);
    item_state = menu_state;
    item_state.selected_item = item_index;
    item = loetregler_current_menu_item(&item_state);
    loetregler_menu_value_text(&settings, item, value_text, sizeof(value_text));

    snprintf(line_buffer,
             sizeof(line_buffer),
             "%c%c %-8s %s",
             item_index == menu_state.selected_item ? '>' : ' ',
             (item_index == menu_state.selected_item && menu_state.editing_value) ? '*' : ' ',
             loetregler_menu_label(item),
             value_text);

    ssd1306_draw_string(&display, 0, (uint8_t) (8U + row * 8U), 1, line_buffer);
  }

  ssd1306_show(&display);
}

static void draw_cooling_screen(const char *reason) {
  char line2[21];
  char line3[21];

  snprintf(line2, sizeof(line2), "Grund: %s", reason);
  snprintf(line3, sizeof(line3), "Spitze: %3uC", runtime_state.tip_temperature_c);
  draw_text_screen("ACHTUNG HEISS", line2, line3, "Warte auf <60C");
}

static void enter_menu(void) {
  runtime_state.screen = SCREEN_MENU;
  menu_state.selected_item = 0;
  menu_state.editing_value = false;
}

static void leave_menu(void) {
  runtime_state.screen = SCREEN_MAIN;
}

static void save_settings_and_clamp_runtime(void) {
  // Nach jeder Menueaenderung werden die Werte sofort:
  // 1. begrenzt,
  // 2. in den Laufzeitdaten uebernommen,
  // 3. dauerhaft im Flash gespeichert.
  loetregler_sanitize_settings(&settings);
  sync_setpoint_with_settings();
  settings_store_save(&settings);
}

static void set_heater_pwm(uint8_t duty_cycle) {
  uint16_t level;

  level = (uint16_t) (((uint32_t) (PWM_WRAP_VALUE + 1U) * duty_cycle) / 255U);
  pwm_set_gpio_level(PIN_HEATER, level);
}

static void request_power_off(const char *reason) {
  runtime_state.screen = SCREEN_COOLING;
  set_heater_pwm(0);

  // Vor dem Ausschalten wird nicht einfach sofort die Versorgung getrennt.
  // Stattdessen wartet die Firmware auf eine sichere Spitzentemperatur.
  // Das macht die Schutzfunktion im Unterricht sichtbar und nachvollziehbar.
  while (true) {
    button_panel_update(&buttons, now_ms());
    runtime_state.tip_temperature_c = read_tip_temperature_c();
    draw_cooling_screen(reason);

    if (runtime_state.tip_temperature_c < SAFE_SWITCH_OFF_TEMPERATURE_C ||
        runtime_state.tip_temperature_c > TIP_SENSOR_ERROR_TEMPERATURE_C) {
      break;
    }
    sleep_ms(100);
  }

  gpio_put(PIN_POWER_HOLD, 0);
  while (true) {
    sleep_ms(1000);
  }
}

static void handle_main_buttons(void) {
  if (button_take_event(&buttons.up) == BUTTON_EVENT_PRESSED) {
    runtime_state.manual_setpoint_c = loetregler_clamp_setpoint(
        (uint16_t) (runtime_state.manual_setpoint_c + settings.temperature_step_c),
        &settings);
    note_user_activity();
  }

  if (button_take_event(&buttons.down) == BUTTON_EVENT_PRESSED) {
    int next_setpoint;

    next_setpoint = (int) runtime_state.manual_setpoint_c - (int) settings.temperature_step_c;
    runtime_state.manual_setpoint_c = loetregler_clamp_setpoint(
        next_setpoint < 0 ? 0U : (uint16_t) next_setpoint,
        &settings);
    note_user_activity();
  }

  if (button_take_event(&buttons.back) == BUTTON_EVENT_PRESSED) {
    runtime_state.standby = !runtime_state.standby;
    note_user_activity();
  }

  if (button_take_event(&buttons.enter) == BUTTON_EVENT_PRESSED) {
    enter_menu();
    note_user_activity();
  }
}

static void handle_menu_buttons(void) {
  if (button_take_event(&buttons.up) == BUTTON_EVENT_PRESSED) {
    if (menu_state.editing_value) {
      if (loetregler_change_menu_value(&settings, &menu_state, +1)) {
        save_settings_and_clamp_runtime();
      }
    } else {
      loetregler_move_menu_selection(&menu_state, -1);
    }
    note_user_activity();
  }

  if (button_take_event(&buttons.down) == BUTTON_EVENT_PRESSED) {
    if (menu_state.editing_value) {
      if (loetregler_change_menu_value(&settings, &menu_state, -1)) {
        save_settings_and_clamp_runtime();
      }
    } else {
      loetregler_move_menu_selection(&menu_state, +1);
    }
    note_user_activity();
  }

  if (button_take_event(&buttons.enter) == BUTTON_EVENT_PRESSED) {
    menu_enter_result_t result;

    result = loetregler_activate_menu_item(&menu_state, &settings);
    if (result == MENU_ENTER_SETTINGS_CHANGED) {
      runtime_state.manual_setpoint_c = settings.startup_temperature_c;
      save_settings_and_clamp_runtime();
    } else if (result == MENU_ENTER_CLOSE_MENU) {
      leave_menu();
    }
    note_user_activity();
  }

  if (button_take_event(&buttons.back) == BUTTON_EVENT_PRESSED) {
    if (loetregler_menu_back_leaves_menu(&menu_state)) {
      leave_menu();
    }
    note_user_activity();
  }
}

static void handle_buttons(void) {
  if (button_take_event(&buttons.power) == BUTTON_EVENT_PRESSED) {
    runtime_state.forced_shutdown = false;
    request_power_off("Taster");
  }

  if (runtime_state.screen == SCREEN_MENU) {
    handle_menu_buttons();
  } else {
    handle_main_buttons();
  }
}

static void update_measurements(void) {
  runtime_state.battery_voltage_v = read_battery_voltage();
  runtime_state.tip_temperature_c = read_tip_temperature_c();
  runtime_state.active_target_c = loetregler_active_target_temperature(runtime_state.manual_setpoint_c,
                                                                       runtime_state.standby,
                                                                       &settings);
}

static void update_heater_control(void) {
  uint8_t duty_cycle;

  runtime_state.current_ampere = 0.0f;
  runtime_state.power_watt = 0.0f;

  if (runtime_state.tip_temperature_c >= runtime_state.active_target_c) {
    set_heater_pwm(0);
    return;
  }

  // Die Heizregelung ist absichtlich kein komplizierter PID-Regler.
  // Fuer den Unterricht ist eine einfache Zweistufen-Strategie leichter:
  // weit unter Soll -> stark heizen, knapp darunter -> sanfter heizen.
  duty_cycle = runtime_state.tip_temperature_c + 80U < runtime_state.active_target_c ? PWM_HEATING_FAST : PWM_HEATING_SOFT;
  set_heater_pwm(duty_cycle);
  sleep_ms(duty_cycle == PWM_HEATING_FAST ? 50 : 10);

  runtime_state.current_ampere = read_heater_current_a();
  runtime_state.power_watt = runtime_state.battery_voltage_v * runtime_state.current_ampere * (float) duty_cycle / 255.0f;
  set_heater_pwm(0);
}

static void check_safety_rules(void) {
  // Alle Sicherheitsregeln sind bewusst an einer Stelle gebuendelt.
  // So kann man die Schutzfunktionen beim Lesen des Programms
  // schnell finden und separat besprechen.
  if (runtime_state.battery_voltage_v < BATTERY_EMPTY_VOLTAGE && now_ms() > 10000UL) {
    runtime_state.forced_shutdown = true;
    request_power_off("Akku leer");
  }

  if (runtime_state.tip_temperature_c > TIP_SENSOR_ERROR_TEMPERATURE_C) {
    runtime_state.forced_shutdown = true;
    request_power_off("Sensorfehler");
  }

  if ((now_ms() - runtime_state.last_user_action_ms) > loetregler_shutdown_timeout_ms(&settings)) {
    runtime_state.forced_shutdown = false;
    request_power_off("Zeit abgel.");
  }
}

static void init_hardware(void) {
  stdio_init_all();
  sleep_ms(100);
  printf("HTL LOETREGLER MINI - Pico SDK C\n");

  // Der Pico haelt sich ueber die Selbsthalteschaltung selbst eingeschaltet.
  // Deshalb wird dieser Ausgang direkt beim Start auf HIGH gesetzt.
  gpio_init(PIN_POWER_HOLD);
  gpio_set_dir(PIN_POWER_HOLD, GPIO_OUT);
  gpio_put(PIN_POWER_HOLD, 1);

  gpio_init(PIN_COM_DRIVER_ENABLE);
  gpio_set_dir(PIN_COM_DRIVER_ENABLE, GPIO_OUT);
  gpio_put(PIN_COM_DRIVER_ENABLE, 0);

  // Fuer die Temperatur- und Strommessung werden drei ADC-Kanaele benutzt.
  adc_init();
  adc_gpio_init(GPIO_ADC_BATTERY);
  adc_gpio_init(GPIO_ADC_TIP_SENSOR);
  adc_gpio_init(GPIO_ADC_CURRENT);

  // Das Display haengt per I2C am ersten I2C-Controller des RP2040.
  i2c_init(I2C_PORT, 400000);
  gpio_set_function(PIN_I2C_SDA, GPIO_FUNC_I2C);
  gpio_set_function(PIN_I2C_SCL, GPIO_FUNC_I2C);
  gpio_pull_up(PIN_I2C_SDA);
  gpio_pull_up(PIN_I2C_SCL);
  ssd1306_init(&display, I2C_PORT, 0x3C, SCREEN_WIDTH, SCREEN_HEIGHT);

  // Der MOSFET wird per PWM angesteuert.
  // Frequenz und Wrap-Wert sind absichtlich direkt hier sichtbar,
  // damit man sie im Unterricht einfach erklaeren kann.
  gpio_set_function(PIN_HEATER, GPIO_FUNC_PWM);
  pwm_slice = pwm_gpio_to_slice_num(PIN_HEATER);
  pwm_set_clkdiv(pwm_slice, PWM_CLOCK_DIVIDER);
  pwm_set_wrap(pwm_slice, PWM_WRAP_VALUE);
  pwm_set_enabled(pwm_slice, true);
  set_heater_pwm(0);

  button_panel_begin(&buttons);
}

int main(void) {
  init_hardware();

  settings = loetregler_default_settings();
  settings_store_load(&settings);
  runtime_state.manual_setpoint_c = settings.startup_temperature_c;
  runtime_state.last_user_action_ms = now_ms();
  runtime_state.screen = SCREEN_MAIN;
  runtime_state.standby = false;
  runtime_state.forced_shutdown = false;

  show_greeting_screen();

  while (true) {
    button_panel_update(&buttons, now_ms());
    handle_buttons();
    update_measurements();
    update_heater_control();
    check_safety_rules();

    if (runtime_state.screen == SCREEN_MENU) {
      draw_menu_screen();
    } else {
      draw_main_screen();
    }

    gpio_put(PIN_POWER_HOLD, 1);
    sleep_ms(20);
  }
}
