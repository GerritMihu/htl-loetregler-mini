#ifndef SETTINGS_STORE_H
#define SETTINGS_STORE_H

#include <stdbool.h>
#include <LoetreglerLogic.h>

bool settings_store_load(loetregler_settings_t *settings);
bool settings_store_save(const loetregler_settings_t *settings);

#endif
