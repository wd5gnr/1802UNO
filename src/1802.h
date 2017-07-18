#ifndef __1802_H
#define __1802_H
#include <stdint.h>

// Configuration
#define MAXMEM 0x3FF  // maximum memory address; important: only 1K of EEPROM to store stuff in
#define LED_PORT 4     // port for DATA LED display
#define SW_PORT 4      // Front panel switch port
#define SER_INP 1     // UART input port
#define SER_OUT 1     // UART output port
#define CTL_PORT 7    // Control port
#define A0_PORT 2     // LSD address display output port
#define A1_PORT 3     // MSD address display output port

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
extern const uint8_t rom[];
extern uint16_t rombase;
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

#endif
