#include <stdio.h>
#include <math.h>
#include "sweetie.h"

int main() {

  for(int i=0;i<16;i++) {
    printf("%2d,%2d,%2d,\n",
      (int)floor((double)sweetie_palette[i*3+0]/255*31),      
      (int)floor((double)sweetie_palette[i*3+1]/255*31),      
      (int)floor((double)sweetie_palette[i*3+2]/255*31)      
    );
  }

  return 0;
}
