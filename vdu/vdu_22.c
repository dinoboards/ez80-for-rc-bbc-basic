#include "../bbcbasic.h"
#include "../vdu.h"
#include <v99x8.h>

// VDU: 22 (1 byte)

// MODE 0: 640x256 graphics, 80x32 characters, 2 colours, 20kB RAM
// MODE 1: 320x256 graphics, 40x32 characters, 4 colours, 20kB RAM
// MODE 2: 160x256 graphics, 20x32 characters, 16 colours, 20kB RAM
// MODE 3: no graphics, 80x25 characters, 2 colours, 16kB RAM
// MODE 4: 320x256 graphics, 40x32 characters, 2 colours, 10kB RAM
// MODE 5: 160x256 graphics, 20x32 characters, 4 colours, 10kB RAM
// MODE 6: no graphics, 40x25 characters, 2 colours, 8kB RAM
// MODE 7: teletext, 40x25 characters, 8 colours, 1kB RAM

// BBC MODE   | V9958 MODE
//    xx      | MULTI-COLOR   64x48 16 colours                            8,9
//    xx      | G1            32*8x24*8 16 colours only 256 patterns      10,11
//    xx      | G3            32*8x24*8 16 colours 768 patterns           12
//    xx      | G4 (212)      256x212 16 colours    => 2,5                13
//    xx      | G4 (192)      256x192 16 colours                          15
//    xx      | G5 (212)      512x212 4 colours     => 0,1,4              16
//    xx      | G5 (192)      512x192 4 colours                           17
//    xx      | G6 (212)      512x212 16 colours                          18
//    xx      | G6 (192)      512x192 16 colours                          19
//    xx      | G7 (212)      256x212 256 colours                         20
//    xx      | G7 (192)      256x192 256 colours                         21

//    xx      | xx - serial I/O  MODE 255 (-1)

// VDU 22 This VDU code is used to change MODE. It is followed by one number which
// is the new mode. Thus VDU 22,7 is exactly equivalent to MODE 7 (except that
// it does not change HIMEM).

void vdu_mode() {
  vdp_set_lines(212);
  current_display_mode = data[0];
  last_text_row        = 26;
  tviewport.left       = 0;
  tviewport.bottom     = 26;
  tviewport.top        = 0;

  switch (data[0]) {
  case 0:
    vdp_set_palette(default_2_colour_palette);
    current_tfg_colour       = 1;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 1;
    last_text_column         = 63;
    tviewport.right          = 63;
    vdp_set_graphic_5();
    break;

  case 1:
    vdp_set_palette(default_4_colour_palette);
    current_tfg_colour       = 3;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 3;
    last_text_column         = 63;
    tviewport.right          = 63;
    vdp_set_graphic_5();
    break;

  case 4:
    vdp_set_palette(default_2_colour_palette);
    current_tfg_colour       = 1;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 1;
    last_text_column         = 63;
    tviewport.right          = 63;
    vdp_set_graphic_5();
    break;

  case 2:
    vdp_set_palette(default_16_colour_palette);
    current_tfg_colour       = 7;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 15;
    last_text_column         = 31;
    tviewport.right          = 31;
    vdp_set_graphic_4();
    break;

  case 5:
    vdp_set_palette(default_4_colour_palette);
    current_tfg_colour       = 3;
    current_tbg_colour       = 0;
    current_mode_colour_mask = 3;
    last_text_column         = 31;
    tviewport.right          = 31;
    vdp_set_graphic_4();
    break;

  case 255:
    break;

  default:
    vdu_not_implemented();
  }

  gsviewport.left   = convert_x(gviewport.left);
  gsviewport.top    = convert_y(gviewport.top);
  gsviewport.right  = convert_x(gviewport.right);
  gsviewport.bottom = convert_y(gviewport.bottom);

  vdu_cls();
}
