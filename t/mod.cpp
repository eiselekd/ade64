#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../ade64.cpp"
#include "u.cpp"

int main(int argc, char **argv) {
  unsigned char p[16]; int i;
  char *val = 0;
  memset(p,0,sizeof(p));
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i],"-v") == 0) {
      verbose = 1;
    } else {
      val = argv[i];
    }
  }
  int len = conv(val,p);
  int c = p[0];
  BYTE mod = c & 0xC0;
  BYTE rm  = c & 0x07;
  printf("all: 0x%x\n", c);
  printf("mod: 0x%x\n", mod >> 6);
  printf("rm : 0x%x\n", rm);

  exit(0);
}
