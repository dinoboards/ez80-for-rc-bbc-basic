#include <ez80-firmware.h>
#include <stdint.h>

#include <stdio.h>

int32_t time_offset;

static uint24_t get_centi_time() {
  uint8_t f = ez80_timers_freq_tick_get();

  uint24_t t = ez80_timers_ticks_get();

  return t * 100 / f;
}

uint32_t get_time() { return time_offset + (int32_t)get_centi_time(); }

void set_time(uint32_t new_time) {
  uint24_t t  = get_centi_time();
  time_offset = (int32_t)new_time - (int32_t)t;
}
