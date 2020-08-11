#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"
#include "interruption.h"
#include "sched.h"
#include "syscall.h"
#include "memory.h"

void start(void *SystemTable __attribute__ ((unused)), struct HardwareInfo *_hardware_info) {
  // From here - Put this part at the top of start() function
  // Do not use _hardware_info directry since this data is located in UEFI-app space
  hardware_info = *_hardware_info;
  init_segmentation();
  // To here - Put this part at the top of start() function

  init_frame_buffer(&hardware_info.fb);
  // puts("hello world!!\nI'm Jumpei!!!\rThis is a pen.Are you a pen ?\napplepen pine-apple pen\n");

  // puts("100 is ");
  // puth(100,4);
  // puts("\n");


  init_acpi_pm_timer(hardware_info.rsdp);

  // puts("wait 2 sec\n");

  // pm_timer_wait_millisec(2000);

  // puts("finished\n");

  unsigned int freq = measure_lapic_freq_khz();
  // puts("freq is ");
  // puth(freq,8);
  // puts("\n");

  // unsigned long long int handler;
  // asm volatile ("lea put_hello(%%rip), %[handler]" : [handler]"=r"(handler));
  // lapic_periodic_exec(1000,(void *)handler);
  // lapic_periodic_exec(1000,(void *)put_hello);

  init_virtual_memory();

  void *handler;
  asm volatile ("lea schedule(%%rip), %[handler]":[handler]"=r"(handler));

  lapic_periodic_exec(2000, handler);
  // init_virtual_memory();
  init_tasks();
  // Do not delete it!

  // init_intr();


  // unsigned long long ret;
  // char *str = "hello world\n";
  // asm volatile (
  //   "mov %[id], %%rdi\n"
  //   "mov %[str], %%rsi\n"
  //   "int $0x80\n"
  //   "mov %%rax, %[ret]\n"
  //   : [ret]"=r"(ret)
  //   : [id]"r"((unsigned long long)SYSCALL_PUTS),
  //    [str]"m"((unsigned long long)str));

  while (1);
}
