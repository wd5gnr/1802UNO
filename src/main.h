#ifndef __MAIN_H
#define __MAIN_H

int freeRam(void);
void interpretkeys(void);
void setupUno(void);
uint8_t xkeyPressed(void);
void scanKeys(void);
void driveLEDs(void);
void setaddress(uint16_t a);
void setdata(uint8_t d);
void setdp(int pos, int state);

#define KEY_RS 18
#define KEY_AD 1
#define KEY_DA 4
#define KEY_GO 7
#define KEY_PC 16
#define KEY_ST 20

#endif
