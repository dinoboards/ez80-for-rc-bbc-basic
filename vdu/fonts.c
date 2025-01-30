#include "../vdu.h"
#include <string.h>
#include <v99x8.h>

#include <stdio.h>

void prepare_font_pattern(uint8_t ch, uint16_t gpos_x, uint16_t gpos_y) {
  if (font_color[ch].fg == current_tfg_colour && font_color[ch].bg == current_tbg_colour)
    return;

  uint8_t *p        = &font_patterns[ch * 8];
  font_color[ch].fg = current_tfg_colour;
  font_color[ch].bg = current_tbg_colour;

  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vdp_to_vram(gpos_x, gpos_y, 8, 8, current_tbg_colour, 0, 0);

  for (int y = 0; y < 8; y++) {
    const uint8_t r = *p++;
    for (int x = 0; x < 8; x++) {
      const bool pixel_on = (r & (1 << (7 - x)));
      if (pixel_on) {
        vdp_cmd_wait_completion();
        vdp_cmd_pset(gpos_x, gpos_y, current_tfg_colour, 0);
      }
      gpos_x++;
    }

    gpos_x -= 8;
    gpos_y++;
  }
}

void init_font_patterns() {
  memset(font_patterns, 0, 256 * 8);

  memcpy(&font_patterns[' ' * 8], sysfont, sizeof(sysfont));

  for (int i = 0; i < 256; i++) {
    font_color[i].fg = 255;
    font_color[i].bg = 255;
  }
}
