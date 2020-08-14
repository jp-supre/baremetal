#pragma once

void init_nic(unsigned int);

unsigned char send_frame(void *, unsigned short);

unsigned short recieve_frame(void *);