#include "../vdu.h"
#include <v99x8.h>

#include <stdio.h>

void mode_4_preload_fonts() {

  // erase all of the page, so we only need to write the on dots

  vdp_cmd_wait_completion();
  vdp_cmd_logical_move_vdp_to_vram(0, 256, vdp_get_screen_width(), vdp_get_screen_height(), current_tbg_colour, 0, 0);

  uint8_t *p = sysfont;

  const uint8_t starting_ch = ' ';
  uint16_t      gpos_x      = (starting_ch % 32) * 8;
  uint16_t      gpos_y      = 256 + (starting_ch / 32) * 8;

  for (int ch_row = ' '; ch_row < 127; ch_row += 32) {
    for (int ch_col = 0; ch_col < 32; ch_col++) {
      for (int y = 0; y < 8; y++) {
        uint8_t r = *p++;
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

      gpos_x += 8;
      gpos_y -= 8;
    }
    gpos_x = 0;
    gpos_y += 8;
  }
}

void preload_fonts() {
  switch (current_display_mode) {
  case 0:
  case 1:
  case 4:
    mode_5_preload_fonts();
    return;

  case 2:
  case 5:
    mode_4_preload_fonts();
    return;

  default:
    printf("todo: preload fonts\r\n");
  }
}
