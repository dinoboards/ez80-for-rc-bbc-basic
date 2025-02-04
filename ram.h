#ifndef __RAM
#define __RAM

#include <stdint.h>

extern uint8_t *HIMEM;
extern uint8_t *LOMEM;
extern uint8_t  CPM_SYS_FCB[36];
extern uint8_t  DISK_BUFFER[128];
extern uint8_t  ACCS[256];
extern uint8_t  BUFFER[256];
extern uint8_t  OPTVAL;
extern uint8_t *TABLE[8];
extern uint8_t  TRPCNT;
extern uint8_t  RAM_END[];
// extern uint8_t *USER;
extern uint8_t  end_of_bss[];
extern uint8_t  _heap[];
extern uint32_t STAVAR[27];
extern uint24_t FREE;
extern uint8_t  FCB_BLOCKS[];
extern uint24_t FREE_FCB_TABLE;

#endif
