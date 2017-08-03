#include "ihex.h"

  
// helper for checksum calculation
  uint8_t ihexread::cksumfin(uint8_t cksum)
  {
    return (~cksum)+1;
  }

// Read 1 hex digit, -1 on error
int ihexread::hex1()
{
  int c;
  c=getch();
  if (c==-1) return -1;
  if (c>='0'&&c<='9') c-='0';
  if (c>='A'&&c<='F') c=(c-'A')+10;
  if (c>='a'&&c<='f') c=(c-'a')+10;
  return c;
}


// read two hex digits, -1 on error
int ihexread::hex2()
{
  int v=0;
  int c;
  v=hex1();
  if (v==-1) return -1;
  v<<=4;
  c=hex1();
  if (c==-1) return -1;
  v|=c;
  return v;
}

    

// Read hex file
  int ihexread::read()
  {
    // character c, count n, value v, address a, checksum cksum)
    int c,i;
    unsigned int eof=0,n,v,a,cksum=0;
    while (!eof)
      {
	c=getch();
	if (c==-1) eof=1;
	if (eof==1) break;  // EOF ok here
	if (c!=':') continue;  // wait for :
	n=hex2();  // read count
	cksum=n;
	a=hex2();// read address
	cksum+=a;
	a<<=8;
	v=hex2();   
	a|=v;
	cksum+=v;  // read record type
	v=hex2();
	cksum+=v;
	// v is now record type
	// we only look at 00 and 01, but
	// you could do more here
	if (v==1)  // end?
	  {
	    // you could read the rest here which should be ff
	    v=hex2();
	    cksum=cksumfin(cksum);
	    if (v!=cksum) return error(); else return 0;
	  }
	if (v!=0) return error(); // unknown type;
		    // read type 0 line
	for (i=0;i<n;i++)
	  {
	    v=hex2();
	    if (v==-1) return error();
	    cksum+=v;
	    setmem(a++,v);
	  }
	v=hex2();
	cksum=cksumfin(cksum);
	if (v!=cksum) return error();
      }
	
    
  }

// Test program

#if 0
#include <stdio.h>

class ihexreadstdin : public ihexread
{
protected:
  int getch(void);
  void setmem(uint16_t a, uint8_t d);
  
} ;

// example function -- you should supply your own
// in a subclass
  int ihexreadstdin::getch()
  {
    return getchar();
  }

void ihexreadstdin::setmem(uint16_t a, uint8_t d)
{
  printf("Write: %04X:%02x\n",a,d);
}


int main(int argc, char *argv[])
{
  ihexreadstdin reader;
  reader.read();
  return 0;
  
}

#endif
