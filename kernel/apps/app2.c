#include "syscall.h"

void app2() {
  while (1) {
    char *str = "Hello from app2\n";

    // TODO: Say hello from here by issuing `puts` syscall
    syscall_puts(str);

    volatile int i = 100000000;
    while (i--);
  }
}
