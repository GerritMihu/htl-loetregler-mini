#include "settings_store.h"

#include <string.h>
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/stdlib.h"

typedef struct {
  uint32_t magic;
  uint16_t version;
  loetregler_settings_t settings;
  uint16_t checksum;
} stored_settings_block_t;

#define SETTINGS_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static uint16_t calculate_checksum(const stored_settings_block_t *block) {
  const uint8_t *bytes;
  uint16_t checksum;
  size_t index;
  size_t byte_count;

  bytes = (const uint8_t *) block;
  byte_count = sizeof(stored_settings_block_t) - sizeof(block->checksum);
  checksum = 0;

  for (index = 0; index < byte_count; ++index) {
    checksum = (uint16_t) (checksum + bytes[index]);
  }
  return checksum;
}

bool settings_store_load(loetregler_settings_t *settings) {
  const stored_settings_block_t *block;

  // Die Konfiguration liegt im letzten Flash-Sektor.
  // Beim Start lesen wir den Datenblock direkt aus dem XIP-Adressraum.
  block = (const stored_settings_block_t *) (XIP_BASE + SETTINGS_FLASH_OFFSET);
  if (block->magic != LOETREGLER_SETTINGS_MAGIC) {
    *settings = loetregler_default_settings();
    return false;
  }

  if (block->version != LOETREGLER_SETTINGS_VERSION) {
    *settings = loetregler_default_settings();
    return false;
  }

  if (block->checksum != calculate_checksum(block)) {
    *settings = loetregler_default_settings();
    return false;
  }

  *settings = block->settings;
  loetregler_sanitize_settings(settings);
  return true;
}

bool settings_store_save(const loetregler_settings_t *settings) {
  stored_settings_block_t block = {0};
  uint8_t sector_buffer[FLASH_SECTOR_SIZE];
  uint32_t interrupts;

  // Flash kann nicht byteweise ueberschrieben werden.
  // Deshalb bereiten wir immer zuerst einen ganzen Sektor im RAM vor
  // und schreiben ihn dann komplett zurueck.
  memset(sector_buffer, 0xFF, sizeof(sector_buffer));

  block.magic = LOETREGLER_SETTINGS_MAGIC;
  block.version = LOETREGLER_SETTINGS_VERSION;
  block.settings = *settings;
  loetregler_sanitize_settings(&block.settings);
  block.checksum = calculate_checksum(&block);

  memcpy(sector_buffer, &block, sizeof(block));

  interrupts = save_and_disable_interrupts();
  flash_range_erase(SETTINGS_FLASH_OFFSET, FLASH_SECTOR_SIZE);
  flash_range_program(SETTINGS_FLASH_OFFSET, sector_buffer, FLASH_SECTOR_SIZE);
  restore_interrupts(interrupts);

  return true;
}
