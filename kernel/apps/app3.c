#include "syscall.h"

void app3() {
  while (1) {
    char *str = "Hello from app3\n";

    // TODO: Say hello from here by issuing `puts` syscall
    syscall_puts(str);

    volatile int i = 100000000;
    while (i--);
  }
}
