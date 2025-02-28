#include "../vdu.h"
#include <v99x8.h>

void vdu_lf() {
  current_tpos.y++;

  if (current_tpos.y >= tviewport.bottom) {
    current_tpos.y--;

    uint16_t left   = (uint16_t)tviewport.left * 8;
    uint16_t top    = (uint16_t)tviewport.top * 8;
    uint16_t right  = (uint16_t)tviewport.right * 8;
    uint16_t bottom = (uint16_t)tviewport.bottom * 8;

    uint16_t width  = right - left + 8;
    uint16_t height = bottom - top;

    // TODO: ONLY SCROLL IF TEXT CURSOR IS ACTIVE

    vdp_cmd_wait_completion();
    vdp_cmd_move_vram_to_vram(left, top + 8, left, top, width, height, 0);
    vdp_cmd_wait_completion();
    vdp_cmd_vdp_to_vram(left, bottom - 8, width, 8, current_tbg_colour, 0);
  }
}
