#include "hardware.h"
#include "segmentation.h"
#include "util.h"

struct FrameBuffer *FB;

unsigned int fb_y,fb_x;

void init_frame_buffer(struct FrameBuffer *fb){
  
  FB = fb;

 for (unsigned int i = 0;i < FB->height; i++){
    for (unsigned int j = 0;j < FB->width; j++){
      struct Pixel *pixel = FB->base + FB->width*i + j;
      pixel->r = 0;
      pixel->g = 0;
      pixel->b = 0;
    }
  }
 fb_y = fb_x = 0;
 }

static void putc(char c){
  unsigned char mask = 1;
  for (unsigned char i = 0;i < 8;i++) {
    for (unsigned char j = 0;j < 8;j++) {
      struct Pixel *pixel = FB->base + FB->width*fb_y + fb_x;
      if (((font[(unsigned char)c][i] >> (7 - j)) & mask) == mask){
	     pixel->r = 255;
	     pixel->g = 255;
	     pixel->b = 255;
      }
      fb_x += 1;
    }
    fb_x -= 8;
    fb_y += 1;
  }
  fb_x += 8;
  fb_y -= 8;
}

static void carridge_ret(){
  for (unsigned int i = 0;i < 8;i++) {
    for (unsigned int j = 0;j < FB->width;j++) {
      struct Pixel *pixel = FB->base + FB->width*fb_y + fb_x;
      pixel->r = 0;
      pixel->g = 0;
      pixel->b = 0;
      fb_x += 1;
    }
    fb_x -= FB->width;
    fb_y += 1;
  }
}

void puts (char *str) {
  char c = *str;
  unsigned int p = 0;
  while(c != 0x00){
    if (c == 0x0a){
      fb_y += 8;
      fb_x = 0;
    } else if (c == 0x0d){
      fb_x = 0;
      carridge_ret();
      fb_y -= 8;
    } else {
      putc(c);
    }
    p++;
    c = *(str+p);
  }
}

void puth(unsigned long long value,unsigned char digit_len){
  char display[digit_len];
  for (unsigned char i = 0;i < digit_len;i++){
    if (value % 16 < 10) {
      display[digit_len-i-1] = 48 + (value % 16);
    } else {
      display[digit_len-i-1] = 65 + (value % 16 -10);
    }
    value /= 16;
  }

  for (unsigned char i = 0;i < digit_len;i++){
    putc(display[i]);
  }
}


unsigned char strncomp(char *s1,char *s2) {
  for (unsigned int i = 0;i<4;i++) {
    if (*(s1+i) != *(s2+i)) {
      return 1;
    }
  }
  return 0;
}

void put_hello() {
  puts("hello world\n");
  return;
}