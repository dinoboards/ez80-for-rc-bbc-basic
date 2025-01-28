#include "../bbcbasic.h"
#include "../vdu.h"
#include <string.h>

#include <stdio.h>
/*
VDU 23,32 to 255,n1,n2,n3,n4,n5,n6,n7,n8
These redefine the printable ASCII characters. The bit pattern of each of the
parameters n1 to n8 corresponds to a row in the eight-by-eight grid of the
character
*/

static void vdu_redefine_font() {

  printf("vdu_redefine_font: %x, %x, %x", data[0], data[1], data[2]);
  // calculate x and y of 'ch'

  uint8_t *src  = &data[1];
  uint8_t *dest = &font_patterns[data[0] * 8];
  memcpy(dest, src, 8);

  uint16_t       gpos_x = (data[0] % 32) * 8;
  uint16_t       gpos_y = 256 + (data[0] / 32) * 8;
  const uint8_t *p      = &data[1];

  for (int y = 0; y < 8; y++) {
    const uint8_t r = *p++;
    for (int x = 0; x < 8; x++) {
      const bool pixel_on = (r & (1 << (7 - x)));
      if (pixel_on) {
        vdp_cmd_wait_completion();
        vdp_cmd_pset(gpos_x, gpos_y, current_tfg_colour, 0);
      } else {
        vdp_cmd_wait_completion();
        vdp_cmd_pset(gpos_x, gpos_y, current_tbg_colour, 0);
      }

      gpos_x++;
    }

    gpos_x -= 8;
    gpos_y++;
  }
}

/*
VDU 23 is a multi-purpose command taking nine parameters, of which the first
identifies a particular function. Each of the available functions is described below.
Eight additional parameters are required in each case
*/
void vdu_multi_purpose() {

  if (data[0] >= 32) {
    vdu_redefine_font();
    return;
  }

  vdu_not_implemented();
}
