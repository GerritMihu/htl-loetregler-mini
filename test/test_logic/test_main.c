#include <unity.h>

#include <LoetreglerLogic.h>

void test_defaults_are_valid(void) {
  loetregler_settings_t settings;

  settings = loetregler_default_settings();
  TEST_ASSERT_GREATER_OR_EQUAL_UINT16(50, settings.startup_temperature_c);
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(settings.max_temperature_c, settings.startup_temperature_c);
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(settings.max_temperature_c, settings.standby_temperature_c);
  TEST_ASSERT_EQUAL_UINT16(10, settings.temperature_step_c);
}

void test_setpoint_is_limited_by_max_temperature(void) {
  loetregler_settings_t settings;

  settings = loetregler_default_settings();
  settings.max_temperature_c = 300;
  loetregler_sanitize_settings(&settings);

  TEST_ASSERT_EQUAL_UINT16(300, loetregler_clamp_setpoint(420, &settings));
  TEST_ASSERT_EQUAL_UINT16(50, loetregler_clamp_setpoint(20, &settings));
}

void test_menu_navigation_wraps(void) {
  menu_state_t menu_state;

  menu_state.selected_item = 0;
  menu_state.editing_value = false;

  loetregler_move_menu_selection(&menu_state, -1);
  TEST_ASSERT_EQUAL_UINT8(LOETREGLER_MENU_ITEM_COUNT - 1, menu_state.selected_item);

  loetregler_move_menu_selection(&menu_state, +1);
  TEST_ASSERT_EQUAL_UINT8(0, menu_state.selected_item);
}

void test_editing_changes_and_sanitizes_values(void) {
  loetregler_settings_t settings;
  menu_state_t menu_state;
  int index;

  settings = loetregler_default_settings();
  menu_state.selected_item = MENU_ITEM_MAX_TEMPERATURE;
  menu_state.editing_value = true;

  for (index = 0; index < 30; ++index) {
    loetregler_change_menu_value(&settings, &menu_state, -1);
  }

  TEST_ASSERT_EQUAL_UINT16(200, settings.max_temperature_c);
  TEST_ASSERT_LESS_OR_EQUAL_UINT16(settings.max_temperature_c, settings.startup_temperature_c);
}

void test_factory_reset_restores_defaults(void) {
  loetregler_settings_t settings;
  loetregler_settings_t defaults;
  menu_state_t menu_state;
  menu_enter_result_t result;

  settings = loetregler_default_settings();
  defaults = loetregler_default_settings();
  settings.startup_temperature_c = 250;
  settings.greeting_index = 3;

  menu_state.selected_item = MENU_ITEM_FACTORY_RESET;
  menu_state.editing_value = false;

  result = loetregler_activate_menu_item(&menu_state, &settings);

  TEST_ASSERT_EQUAL_INT(MENU_ENTER_SETTINGS_CHANGED, result);
  TEST_ASSERT_EQUAL_UINT16(defaults.startup_temperature_c, settings.startup_temperature_c);
  TEST_ASSERT_EQUAL_UINT8(defaults.greeting_index, settings.greeting_index);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_defaults_are_valid);
  RUN_TEST(test_setpoint_is_limited_by_max_temperature);
  RUN_TEST(test_menu_navigation_wraps);
  RUN_TEST(test_editing_changes_and_sanitizes_values);
  RUN_TEST(test_factory_reset_restores_defaults);
  return UNITY_END();
}
