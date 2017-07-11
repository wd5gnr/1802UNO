#include <stdio.h>

int main(int argc, char *argv[])
{
  int byte;
  FILE *bin;
  int group=0;
  
  if (argc!=2)
    {
      fprintf(stderr,"Usage: binto1802 binaryfile\n");
      return 1;
    }
  bin=fopen(argv[1],"rb");
  if (!bin)
    {
      perror(argv[1]);
      return 2;
    }
  printf("@0000:\n");
  while ((byte=getc(bin))!=EOF)
    {
      printf("%02X ",byte);
      if (++group==16)
	{
	  group=0;
	  printf("\n");
	}
    }
  printf(".\n");
  fclose(bin);
  return 0;
}

    
