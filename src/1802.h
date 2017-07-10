#ifndef __1802_H
#define __1802_H
void reset(void);  // reset
int run(void);  // do an instruction, 0 means stop, negative means error (none yet)
int exec1802(int ch);  // do a loop cycle
extern int noserial;
#endif
