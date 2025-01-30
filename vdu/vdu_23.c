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
  uint8_t *src  = &data[1];
  uint8_t *dest = &font_patterns[data[0] * 8];
  memcpy(dest, src, 8);
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
