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
  struct disasm64_struct d;
  int dlen = ade64_disasm(p, &d);
  printf("%d",dlen);
  exit(0);
}
