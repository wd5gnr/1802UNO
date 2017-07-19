#ifndef __MAIN_H
#define __MAIN_H


// How many cycles between display refreshes?
// Higher makes the simulation faster, but the screen more blinky
#define DISPLAY_DIVISOR 32  // number of ticks between display refresh

int freeRam(void);
void interpretkeys(void);
void setupUno(void);
uint8_t xkeyPressed(void);
void scanKeys(void);
void driveLEDs(void);
void setaddress(uint16_t a);
void setdata(uint8_t d);
void setdp(int pos, int state);

#define KEY_RS 'R'
#define KEY_AD '='
#define KEY_DA 'L'
#define KEY_GO 'G'
#define KEY_PC 'P'
#define KEY_ST 'S'
#define KEY_SST '/'

extern char threeHex[3][2];


// Define to 1 for simulation monitor built-in
#define MONITOR 1
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

#endif
