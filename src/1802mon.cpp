#include "1802.h"
#include "main.h"
#if MONITOR == 1
#include <Arduino.h>

/*
Commands:

R - Display all registers
R2 - Display register 2
R2=AA - Set register 2 to AA

Note: X=10, P=11, D=12, DF=13, Q=14, and T=15 -- display all registers to see that list

M 400 - Display 100 hex bytes at 400
M 400 10 - Display 10 hex bytes at 400
M 400=20 30 40; - Set data starting at 400 (end with semicolon)

Note that you can use the first line with full backspace:
M 400=20 30 40;

But if you start a new line (you get a colon prompt), you will not be able to backspace past the current byte:

M 400=
: 20 30 40
: 50 60 70;

Backing up while entering 30 can only delete the 30 and not the 20. Also, instead of backing up you can just keep going 
as in:

:M 400=
: 200 300 400;

All 3 bytes will then be zero.

G 400 - Goto 400
G 400 3 - Goto 400 with P=3

B - List all enabled breakpoints
B0 - - Disable Breakpoint 0 (that is a dash as in B0 -)
BF @200 - Set breakpoint F to address 200
BF P3 - Set breakpoint when P=3 
BF I7A - Set breakpoint when instruction 7A executes 

Note would be possible to do data write (or even read) breakpoints
Would also be possible to do ranges of addresses, if desired

N - Execute next instruction


I 2 - Show input from N=2
O 2 10 - Write 10 to output N=2

Note: The keypad and display are dead while the monitor is in control

X - Exit to front panel mode (not running)

C - Exit and continue running (if already running)
Q - Same as C

.+ - Send next character(s) to the front panel interpreter (no spaces)

*/


static char cmdbuf[33];
static int cb;


static int cmd;        // command
static uint16_t arg;   // one argument
static int terminate;  // termination character
static int noread;


int monactive = 0;


int getch(void) {
  int c;
  int ctr = 0;
  do {
    c = Serialread();
    if (c == -1) {
      if (++ctr % DISPLAY_DIVISOR == 0) {
        updateLEDdata();
        driveLEDs();
        scanKeys();
        ctr = 0;
      }
    }

  } while (c == -1);
  return c;
}


// This skips leading blanks, makes all internal whitespace one space, and trims trailing blanks
// This is very important since some of the parsing assumes there will be 0 or 1 space but no more
// and no tabs etc.

uint8_t readline(int *terminate) {
  int c;
  cb = 0;
  cmdbuf[0] = '\0';
  while (1) {
    c = getch();
    if (c == '\r') {
      if (terminate) *terminate = '\r';
      while (cb && cmdbuf[cb - 1] == ' ') cb--;
      cmdbuf[cb] = '\0';
      if (cb) cb = 1;
      return *cmdbuf;
    }
    if (c == 0x1b) {
      cmdbuf[0] = '\0';
      cb = 0;
      if (terminate) *terminate = 0x1b;
      return '\0';
    }
    if (c == 8 && cb != 0) {
      cb--;
      continue;
    }
    if (cb > sizeof(cmdbuf) - 2) {
      Serial.print('\x7');  // sound bell
      continue;
    }

    if (isspace(c) && cb == 0) continue;
    if (cb && isspace(c) && cmdbuf[cb - 1] == ' ') continue;
    else if (isspace(c)) c = ' ';
    cmdbuf[cb++] = toupper(c);
  }
}



uint16_t readhexX(int (*getcfp)(void), int *term, uint16_t def = 0xFFFF) {
  uint16_t val = def;
  int first = 1;
  int c;

  while (1) {
    c = getcfp();
    if (!isxdigit(c) && c != 8) {
      if (first && c != '\r' && c != ';' && c != 0x1b) continue;
      if (term) *term = c;
      noread = first;
      return val;
    }
    if (c == 8) {
      val >>= 4;  // in case of serial input
      continue;
    }
    c = toupper(c);
    if (first) val = 0;
    else val <<= 4;
    first = 0;
    if (c >= 'A') c = c - 'A' + 10;
    else c -= '0';
    val += c;
  }
}

uint16_t readhex(int *term, uint16_t def = 0xFFFF) {
  return readhexX(getch, term, def);
}

int getbufc(void) {
  if (cmdbuf[cb] == '\0') return '\r';
  return cmdbuf[cb++];
}


uint16_t readhexbuf(int *term, uint16_t def = 0xFFFF) {
  return readhexX(getbufc, term, def);
}



BP bp[16];

void dispbp(int bpn) {
  Serial.print(F("\r\nBP"));
  Serial.print(bpn, HEX);
  Serial.print(F(": "));
  if (bp[bpn].type == 1) Serial.print(F("@"));
  if (bp[bpn].type == 2) Serial.print(F("P"));
  if (bp[bpn].type == 3) Serial.print(F("I"));
  if (bp[bpn].type == 0) Serial.print(F(" DISABLED"));
  else print4hex(bp[bpn].target);
}


int nobreak;

void mon_status(void) {
  print4hex(reg[p]);
  Serial.print(F(": "));
  print2hex(memread(reg[p]));
  Serial.print(F(" D="));
  print2hex(d);
  Serial.println();
}



int mon_checkbp(void) {
  int i;
  int mon = 0;
  if (nobreak) return 1;
  for (i = 0; i < sizeof(bp) / sizeof(bp[0]); i++) {
    if (bp[i].type == 0) continue;
    else if (bp[i].type == 1 && bp[i].target == reg[p]) mon = 1;
    else if (bp[i].type == 2 && bp[i].target == p) mon = 1;
    else if (bp[i].type == 3 && bp[i].target == memread(reg[p])) mon = 1;
  }
  if (mon) {
    Serial.println(F("\nBreak"));
    mon_status();
    return monitor();
  }
  return 1;
}

// dump printable characters
static void adump(unsigned a)
{
  int z;
  Serial.print(F("  "));
  for (z = 0; z < 16; z++)
  {
    char b = memread(a + z);
    if (b >= ' ')
      Serial.print(b);
    else
      Serial.print('.');
  }
}

int monitor(void) {
  int noarg;
  monactive = 1;
  while (1) {
    Serial.print(F("\r\n>"));
    cmd = readline(&terminate);
    if (terminate == 0x1b) continue;
    if (!strchr("RMGBIOXQCN?.", cmd)) {
      Serial.print('?');
      continue;
    }
    noarg = 0;
    if (cmdbuf[cb]) arg = readhexbuf(&terminate);
    else noarg = 1;

    switch (cmd) {
      case '.':
        for (char *cp = cmdbuf + 1; *cp; cp++)
          exec1802(*cp);
        break;

      case '?':
        Serial.println(F("<R>egister, <M>emory, <G>o, <B>reakpoint, <N>ext, <I>nput, <O>utput, e<X>it, <Q>uit, <C>ontinue, .cccc (send characters to front panel; no space after .)"));
        Serial.print(F("Examples: R   RB   RB=2F00   M 100 10    M 100=<CR>AA 55 22;    B 0 @101   .44$$"));
        break;

      case 'N':
        nobreak = 1;
        mon_status();
        run();
        nobreak = 0;
        break;

      case 'B':
        if (noarg || arg >= 0x10) {
          int i;
          for (i = 0; i < sizeof(bp) / sizeof(bp[0]); i++) dispbp(i);
          break;
        }
        if (terminate != '\r') {
          int cc;
          cc = terminate;
          if (cc == ' ') cc = getbufc();
          if (cc == '-') {
            bp[arg].type = 0;
            break;
          }
          if (cc == '@') {
            cc = readhexbuf(&terminate);
            bp[arg].target = cc;
            bp[arg].type = 1;
            break;
          }
          if (cc == 'p' || cc == 'P') {
            cc = readhexbuf(&terminate);
            bp[arg].target = cc & 0xF;
            bp[arg].type = 2;
            break;
          }
          if (cc == 'i' || cc == 'I') {
            cc = readhexbuf(&terminate);
            bp[arg].target = cc & 0xFF;
            bp[arg].type = 3;
            break;
          }


        } else {
          dispbp(arg);
        }
        break;


      case 'R':
        if (noarg) {
          int i;
          for (i = 0; i <= 15; i += 4) {
            Serial.print(F("R"));
            Serial.print(i, HEX);
            Serial.print(':');
            print4hex(reg[i]);
            Serial.print(F("\tR"));
            Serial.print(i + 1, HEX);
            Serial.print(':');
            print4hex(reg[i + 1]);
            Serial.print(F("\tR"));
            Serial.print(i + 2, HEX);
            Serial.print(':');
            print4hex(reg[i + 2]);
            Serial.print(F("\tR"));
            Serial.print(i + 3, HEX);
            Serial.print(':');
            print4hex(reg[i + 3]);
            Serial.println();
          }
          Serial.print(F("(10) X:"));
          Serial.print(x, HEX);
          Serial.print(F("\t(11) P:"));
          Serial.println(p, HEX);
          Serial.print(F("(12) D:"));
          print2hex(d);
          Serial.print(F("\t(13) DF:"));
          Serial.println(df, HEX);
          Serial.print(F("(14) Q:"));
          Serial.print(q, HEX);
          Serial.print(F("\t(15) T:"));
          Serial.print(t, HEX);
        } else {
          if (terminate != '=') Serial.print(F("R"));
          if (arg <= 0xF) {
            if (terminate == '=') {
              uint16_t v = readhexbuf(&terminate);
              reg[arg] = v;
            } else {
              Serial.print(arg, HEX);
              Serial.print(':');
              print4hex(reg[arg]);
            }
          } else {
            switch (arg) {
              case 0x10:
                if (terminate == '=') {
                  uint16_t v = readhexbuf(&terminate);
                  x = v;
                } else {
                  Serial.print(F("X:"));
                  Serial.print(x, HEX);
                }

                break;

              case 0x11:
                if (terminate == '=') {
                  uint16_t v = readhexbuf(&terminate);
                  p = v;
                } else {
                  Serial.print(F("P:"));
                  Serial.print(p, HEX);
                }

                break;

              case 0x12:
                if (terminate == '=') {
                  uint16_t v = readhexbuf(&terminate);
                  d = v;
                } else {
                  Serial.print(F("D:"));
                  Serial.print(d, HEX);
                }

                break;

              case 0x13:
                if (terminate == '=') {
                  uint16_t v = readhexbuf(&terminate);
                  df = v;
                } else {
                  Serial.print(F("DF:"));
                  Serial.print(df, HEX);
                }

                break;

              case 0x14:
                if (terminate == '=') {
                  uint16_t v = readhexbuf(&terminate);
                  q = v;
                } else {
                  Serial.print(F("Q:"));
                  Serial.print(q, HEX);
                }

              case 0x15:
                if (terminate == '=') {
                  uint16_t v = readhexbuf(&terminate);
                  t = v;
                } else {
                  Serial.print(F("T:"));
                  Serial.print(t, HEX);
                }

                break;
            }
          }
        }

        break;

      case 'Q':
        {
          runstate = 0;
          monactive = 0;
          return 0;
        }
      case 'C':
      case 'X': monactive = 0; return 1;
      case 'I':
        print2hex(input(arg));
        break;

      case 'O':
        {
          uint8_t v = readhexbuf(&terminate);
          output(arg, v);
          break;
        }

      case 'G':
        {

          if (terminate != '\r') p = readhexbuf(&terminate);
          reg[p] = arg;
          runstate = 1;
          monactive = 0;
          return 0;
        }

      case 'M':
        {
          uint16_t arg2 = 0;
          if (terminate == '=') {
            uint8_t d;
            while (cmdbuf[cb] != 0) {
              d = readhexbuf(&terminate, 0);
              if (noread == 0) memwrite(arg++, d);
              if (terminate == ';') break;
            }
            if (terminate == ';') break;
            do {
              if (terminate == '\r' || terminate == '=') {
                Serial.print('\n');
                print4hex(arg);
                Serial.print(F(": "));
              }
              d = readhex(&terminate, 0);
              if (terminate != ';' || noread == 0)
                memwrite(arg++, d);
            } while (terminate != ';');
          } else {
            uint16_t i, limit;
            unsigned ct = 16;
            if (terminate != '\r') arg2 = readhexbuf(&terminate, 0);
            if (arg2 == 0) arg2 = 0x100;
            limit = (arg + arg2) - 1;
            if (limit < arg) limit = 0xFFFF;  // wrapped around!
            Serial.print(F("       0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F"));
            for (i = arg; i <= limit; i++)
            {
              if (ct % 16 == 0) {
                if (ct!=16) adump(i - 16);
                Serial.println();
                print4hex(i);
                Serial.print(F(": "));
              } else if (ct % 8 == 0) Serial.print(' ');
              ct++;

              print2hex(memread(i));
              Serial.print(' ');
              if (i == 0xFFFF) break;  // hit limit
              if (Serialread() == 0x1b) break;
            }
            adump(i - 16);
          }
        }
        break;


      default:
        Serial.print((char)cmd);
        Serial.println(F("?"));
    }
  }
}



#endif
