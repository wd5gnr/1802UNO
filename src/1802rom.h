#ifndef __ROM_H
#define __ROM_H

// Remember ROM_SLOT 0 must have the lowest address

// NOTE: Not all 1802 code is address independent (in fact, most probably isn't)
// So the examples below may not work except at address 8000 and are for demo purposes
// only, however they "seem" to work


// Define ROM Base addresses
#define ROM0 0x8000
#define ROM1 0x9000
#define ROM2 0xC000

uint16_t rombase[]=
  {
    ROM0, ROM1, ROM2
  };



// Macros for ROMs to use
#define ROM_ARRAY ROM_ARRAY_(rom,ROM_SLOT)
#define ROM_ARRAY_(a,b) ROM_ARRAY__(a,b)
#define ROM_ARRAY__(a,b) a ## b
#define ROM_BASEVAR ROM_BASEVAR_(rombase,ROM_SLOT)
#define ROM_BASEVAR_(a,b) ROM_BASEVAR__(a,b)
#define ROM_BASEVAR__(a,b) a ## b

// You can see the pattern here: Define ROM_SLOT and ROM_BASE, include the file, and #undef
// Then repeat

#define ROM_SLOT 0
#define ROM_BASE ROM0
#include "1802idiot.h"
#undef ROM_SLOT
#undef ROM_BASE
#define ROM_SLOT 1
#define ROM_BASE ROM1
#include "1802etops.h"
#undef ROM_SLOT
#undef ROM_BASE
#define ROM_SLOT 2
#define ROM_BASE ROM2
#include "1802hilo.h"

// Array of ROMS
uint8_t const *roms[]=
  {
    rom0, rom1, rom2
  };


// Compute size of ROMs
uint16_t romsize[]=
  {
    sizeof(rom0)/sizeof(rom0[0]),
    sizeof(rom1)/sizeof(rom1[0]),    
    sizeof(rom2)/sizeof(rom2[0])
  };
    

#endif



