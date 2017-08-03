#ifndef __1802_H
#define __1802_H
#include <stdint.h>

#include "1802config.h"

// public interface
void reset(void);  // reset
int run(void);  // do an instruction, 0 means stop, negative means error (none yet)
int exec1802(int ch);  // do a loop cycle
extern int noserial;

// Stuff to link with other modules
extern unsigned int idata;


// CPU registers and stuff
extern uint16_t reg[16];
extern uint8_t p,x,d,df,q,t;
extern uint8_t ef1, ef2, ef3, ef4;
extern uint8_t ef4term;

extern uint8_t mp;  // memory protect
// RAM
extern uint8_t ram[MAXMEM+1]; 		// main 1KB RAM		 0x000-0x3FF

// You can have 3 banks of ROMs (easy to add more)
// This bank of ROM must be at lowest address of all ROMs 
extern const uint8_t rom0[];
extern uint16_t rombase0;

extern const uint8_t rom1[];
extern uint16_t rombase1;

extern const uint8_t rom2[];
extern uint16_t rombase2;

extern uint8_t const *roms[];
extern uint16_t rombase[];


extern uint8_t adhigh;  // high address display for I/O
extern uint8_t adlow;  // low address display for I/O
extern int addisp;
extern unsigned int data;
// CPU states... run, load memory, or set address
extern int runstate;
extern int loadstate;
extern int addstate;
extern int tracemode;
extern unsigned int caddress;  // current load address

uint8_t memread(uint16_t a);
void memwrite(uint16_t a, uint8_t d);
uint8_t input(uint8_t port);
void output(uint8_t port, uint8_t val);
void print2hex(uint8_t v);
void print4hex(uint16_t v);
void updateLEDdata(void);

#if MONITOR==1
int monitor(void);
typedef struct
{
  uint8_t type;  // 0 = off, 1=add, 2=p, 3==i
  uint16_t target;
} BP;


extern BP bp[16];

int mon_checkbp(void);
extern int monactive;


#endif



#if BIOS==1
int bios(uint16_t fn);
#endif

#endif
