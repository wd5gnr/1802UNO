/*
Note we use FF01 as a made up SCALL and FF02 as SRET
These should never be transfer control addresses in a real BIOS so...

FF3F - Setup SCRT (jump with ret in R6)
FF2D - Set up baud rate
FF66 - Print string
FF03 - Send char with 0ch translation
FF4e - Send char
f809 - Send char


*/


#include <Arduino.h>
#include "main.h"
#include "1802.h"


#if BIOS == 1


int bios(uint16_t fn) {
  switch (fn) {
    case 0xFF01:  // made up SCALL
      {
        uint16_t cv;
        // Some code depends on RE.0 = D after a CALL or RETURN
        reg[0xe] = (reg[0xe] & 0xFF00) | (d & 0xFF);

        cv = memread(reg[3]) << 8;
        cv |= memread(reg[3] + 1);
        x = 2;
        memwrite(reg[2], reg[6]);
        memwrite(reg[2] - 1, reg[6] >> 8);
        reg[2] -= 2;
        reg[6] = reg[3] + 2;
        reg[3] = cv;
        p = 3;
        reg[4] = 0xFF01;  // reset for next call
      }

      break;

    case 0xFF02:  //  made us RET
      // Some code depends on RE.0 = D after a CALL or RETURN
      reg[0xe] = (reg[0xe] & 0xFF00) | (d & 0xFF);
      reg[2]++;
      reg[3] = reg[6];
      reg[6] = memread(reg[2]) << 8;
      reg[6] |= memread(reg[2] + 1);
      reg[2]++;
      p = 3;
      reg[5] = 0xFF02;
      x = 2;
      break;

    case 0xFF3F:  // set up SCRT
      reg[4] = 0xFF01;
      reg[5] = 0xFF02;
      reg[3] = reg[6];  // assume this wasn't a proper call because we were not set up yet...
      p = 3;
      break;


    case 0xFF2d:                   // Baud rate, not needed here
      reg[0xe] = reg[0xe] & 0xFF;  // select UART
      reg[0xe] |= 0x100;           // turn on echo by default
      p = 5;
      break;

    case 0xFF6C:  // serial break
      df = 0;     // for now no break. TODO: You could read a character to a single byte buffer every time and patch read and check to look there first before querying hardware then you could see if ^C was waiting
      p = 5;
      break;

    case 0xFF66:  // print a string
      {
        char c;
        do {
          c = memread(reg[6]++);
          if (c) Serial.print(c);
        } while (c);
        p = 5;
      }
      break;

    case 0xFF03:
    case 0xFF4e:
    case 0xf809:
      {
        char c = d;
        if (c == 0xC && (fn == 0xf809 || fn == 0xFF03)) {
          Serial.print("\x1b[2J");
        } else
          Serial.print(c);
        p = 5;
      }
      break;



    case 0xf80c:  // uread
    case 0xFF06:
      {
        int c;
        do {

          c = Serial.read();
          if (c > 0 && (reg[0xe] & 0x100)) {
            char echo = c;
            Serial.print(echo);
          }
        } while (c == -1);
        d = c;
        p = 5;
      }
      break;


    case 0xFF09:
      {
        char c;
        do {
          c = memread(reg[0xf]++);
          if (c) Serial.print(c);
        } while (c);
        p = 5;
      }
      break;

    case 0xFF81:
      reg[0xf] = 8;
      p = 5;
      break;

    case 0xf80f:  // key avail?
      df = Serial.available() ? 1 : 0;
      p = 5;
      break;


    case 0xFF0F:
    case 0xFF69:
      {
        int c;
        int n = 0;
        int l = 254;
        uint16_t ptr = reg[0xF];
        char echo;

        if (fn == 0xFF69) l = reg[0xc] - 1;
        do {
          do {
            c = Serial.read();
            echo = c;
          } while (c == -1 || c == 0);
          if (c == 0xD) {
            c = 0;
            df = 0;
          }
          if (c == 3) {
            c = 0;
            df = 1;
          }
          if (c == 8 || c == 0xFF) {
            if (n) {
              ptr--;
              n--;
              if (reg[0xe] & 0x100) Serial.print("\x8 \x8"); 
            }
            continue;
          }
          if (c && n == l) continue;
          memwrite(ptr++, c);
          n++;
          if (c >=0 && (reg[0xe] & 0x100)) Serial.print(echo);
        } while (c != 0);

        p = 5;
      }

      break;
    
      // NOTE MUST REDO THIS ONE... DOES NOT WORK IN ROM!
    case 0xFF12:
      {
#if 0
	  char *p1=ram+reg[0xf];
	  char *p2=ram+reg[0xd];
	  d=strcmp(p1,p2);
	  p=5;
#else
        char p1, p2;
        d = 0;
        do {
          p1 = memread(reg[0xf]);
          p2 = memread(reg[0xd]);
          if (p1 == p2 && p1 == 0) break;  // strings equal
          if (p1 == 0 || p1 < p2) {
            d = 0xFF;
            break;
          }  // I think I got these right
          if (p2 == 0 || p1 > p2) {
            d = 1;
            break;
          }
          reg[0xf] = reg[0xf] + 1;
          reg[0xd] = reg[0xd] + 1;
        } while (1);
        p = 5;
#endif
      }

      break;


    case 0xFF15:
      {
        char c;
        do {
          c = memread(reg[0xf]++);
        } while (c && isspace(c));
        reg[0xf]--;
        p = 5;
      }
      break;

    case 0xFF18:
      {
        char c;
        do {
          c = memread(reg[0xf]++);
          memwrite(reg[0xd]++, c);
        } while (c);
      }
      p = 5;

      break;

    case 0xFF1B:
      while (reg[0xC]--)
        memwrite(reg[0xd]++, reg[0xF]++);
      p = 5;
      break;

    case 0xFF54:  // not sure this really works as expected
      monitor();
      p = 5;
      break;


    case 0xFF57:
      reg[0xf] = 0x7EFF;  // last address (ROM takes 7F00 as worksapce)
      p = 5;
      break;


    default:
      return 0;
  }

  return 1;
}


#endif
