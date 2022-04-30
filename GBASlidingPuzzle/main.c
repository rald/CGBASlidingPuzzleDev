#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "sweetie.h"
#include "font.h"
#include "image.h"


typedef unsigned char      uint8;
typedef unsigned short     uint16;
typedef unsigned int       uint32;

#define REG_DISPLAYCONTROL *((volatile uint32*)(0x04000000))
#define VIDEOMODE_3         0x0003
#define BGMODE_2            0x0400

#define SCREEN ((volatile uint16*)0x06000000)
#define SCREEN_W            240
#define SCREEN_H            160

#define SCREEN_S (SCREEN_W*SCREEN_H)

volatile unsigned short* buttons = (volatile unsigned short*) 0x04000130;

#define BUTTON_A (1 << 0)
#define BUTTON_B (1 << 1)
#define BUTTON_SELECT (1 << 2)
#define BUTTON_START (1 << 3)
#define BUTTON_RIGHT (1 << 4)
#define BUTTON_LEFT (1 << 5)
#define BUTTON_UP (1 << 6)
#define BUTTON_DOWN (1 << 7)
#define BUTTON_R (1 << 8)
#define BUTTON_L (1 << 9)

unsigned char button_pressed(unsigned short button) {
  unsigned short pressed = *buttons & button;
  if (pressed == 0) {
    return 1;
  } else {
    return 0;
  }
}

#define REG_VCOUNT (* (volatile uint16*) 0x04000006)
void vsync(void) {
  while (REG_VCOUNT >= 160);
  while (REG_VCOUNT < 160);
}

uint16 rgb16(uint8 red, uint8 green, uint8 blue) {
  return (red & 0x1F) | (green & 0x1F) << 5 | (blue & 0x1F) << 10;
}

uint16 clr(uint8 c) {
  return rgb16(
    sweetie_palette[c*3+0],
    sweetie_palette[c*3+1],
    sweetie_palette[c*3+2]
  );
}

void pset(volatile uint16* srf,uint8 x,uint8 y,uint16 c) {
  srf[y*SCREEN_W+x]=c;
}

uint16 pget(volatile uint16* srf,uint8 x,uint8 y) {
  return srf[y*SCREEN_W+x];
}

void frect(volatile uint16* srf,uint8 x,uint8 y,uint8 w,uint8 h,uint16 c) {
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
          pset(srf,x+i,y+j,c);
        }
    }
}

void drect(volatile uint16* srf,uint8 x,uint8 y,uint8 w,uint8 h,uint16 c) {
  for(int i=x;i<x+w;i++) {
    pset(srf,i,y,c);
    pset(srf,i,y+h-1,c);    
  }
  for(int j=y;j<y+h;j++) {
    pset(srf,x,j,c);
    pset(srf,x+h-1,j,c);    
  }
}

void dbmp(volatile uint16* srf,uint8* bmp,uint8 w,uint8 h,uint8 f,uint8 x,uint8 y,uint8 t) {
  for(int j=0;j<h;j++) {
    for(int i=0;i<w;i++) {
      int k=bmp[i+j*w+f*w*h];
      if(k!=t) {
        pset(srf,x+i,y+j,clr(k));    
      }
    }    
  }
}

void dchr(volatile uint16* srf,uint8* fnt,uint8 w,uint8 h,uint8 x,uint8 y,uint8 s,uint16 c,uint8 f) {
  for(int j=0;j<h;j++) {
    for(int i=0;i<w;i++) {
      int k=fnt[i+j*w+f*w*h];
      if(k!=0) {
        frect(srf,x+i*s,y+j*s,s,s,c);    
      }
    }    
  }
}

int indexOf(char* l,uint8 c) {
  int j=-1;
  for(int i=0;l[i];i++) {
    if(l[i]==c) {
      j=i;
      break;
    }
  }
  return j;
}

void dtxt(volatile uint16* srf,uint8* fnt,uint8 w,uint8 h,uint8 x,uint8 y,uint8 s,uint16 c,char* txt) {
  char* l="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  int xc=x,yc=y;
  for(int i=0;txt[i];i++) {
    int j=indexOf(l,txt[i]);
    if(j!=-1) {
      dchr(srf,fnt,w,h,xc,yc,s,c,j);
    }
    xc+=w*s;
    if(xc+w*s>=SCREEN_W) {
      yc+=h*s;
      if(yc+h*s>=SCREEN_H) break;
    }
  }
}

void bget(volatile uint16 *srf,uint8 x,uint8 y,uint8 w,uint8 h) {
  for(int j=0;j<h;j++) {
    for(int i=0;i<w;i++) {
      srf[i+j*w]=pget(SCREEN,x+i,y+j);
    }
  }  
}

void bput(volatile uint16* srf,uint8 x,uint8 y,uint8 w,uint8 h) {
  for(int j=0;j<h;j++) {
    for(int i=0;i<w;i++) {
      pset(SCREEN,x+i,y+j,srf[i+j*w]);
    }
  }
}

int sgn(int x) {
  return x<0?-1:x>0?1:0;
}

int main() {

  REG_DISPLAYCONTROL = VIDEOMODE_3 | BGMODE_2;

  uint16 delay=1000;
  int s=16;
  volatile uint16 b[s*s];
  uint16 seed=0;
  int x=0,y=0;
  int px=0,py=0;
  int xi=0,yi=0;
  uint8 d=0,pd=0;
  
  frect(SCREEN,0,0,SCREEN_W,SCREEN_H,clr(0));

  dtxt(SCREEN,font_pixels,font_width,font_height,0,0,1,clr(12),"PRESS START BUTTON");

  while(!button_pressed(BUTTON_START)) seed++;
  srand(seed);

  vsync();
  frect(SCREEN,0,0,SCREEN_W,SCREEN_H,clr(0));

  vsync();
  dbmp(SCREEN,image_pixels,image_width,image_height,0,(SCREEN_W-image_width)/2,(SCREEN_H-image_height)/2,255);

  vsync();
  for(int j=0;j<160/s;j++) {
    for(int i=0;i<240/s;i++) {
      drect(SCREEN,i*s,j*s,s,s,clr(12));
    }  
  }

  x=rand()%(240/s)*s;
  y=rand()%(160/s)*s;

  vsync();
  frect(SCREEN,x,y,s,s,clr(0));

  while(1) {

    px=x;
    py=y;
  
    d=rand()%4;

    while(1) {
      switch(d) {
        case 0: xi= 0; yi=-s; break;
        case 1: xi= 0; yi= s; break;
        case 2: xi=-s; yi= 0; break;
        case 3: xi= s; yi= 0; break;
      }
      if( x+xi<0 || x+xi>=SCREEN_W ||
          y+yi<0 || y+yi>=SCREEN_H || 
          d==pd) d=(d+1)%4;
      else break;
    }

    switch(d) {
      case 0: pd=1; break;
      case 1: pd=0; break;
      case 2: pd=3; break;
      case 3: pd=2; break;
    }

    x+=xi;
    y+=yi;

    vsync();
    bget(b,x,y,s,s);

    while(x!=px || y!=py) {

      vsync();
      frect(SCREEN,x,y,s,s,clr(0));

      x+=sgn(px-x);
      y+=sgn(py-y);

      vsync();
      bput(b,x,y,s,s);      

      for(uint16 i=0;i<delay;i++);

    }

    x+=xi;
    y+=yi;
 
    
  }
  
  return 0;
}
