#pragma once

extern unsigned char font[128][8];

void init_frame_buffer(struct FrameBuffer *fb);

void puts(char *c);

void puth(unsigned long long value, unsigned char digits_len);

unsigned char strncomp(char *s1, char *s2);

void put_hello();
