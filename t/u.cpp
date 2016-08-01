#include <stdio.h>
#include <stdlib.h>

int conv(char *a, unsigned char *p) {
  int c,i = 0;
  while (sscanf(a, "%x", &c) && i < 16) {
    //printf("Found %02x\n", c&0xff);
    p[i] = c;
    a += 3;
    i++;
  }
  return i;
}
