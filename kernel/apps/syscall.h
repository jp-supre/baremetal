#pragma once

typedef enum {
  SYSCALL_PUTS,
} SYSCALL;

unsigned long long syscall_puts(char *str);