#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"

void start(void *SystemTable __attribute__ ((unused)), struct HardwareInfo *_hardware_info) {
  // From here - Put this part at the top of start() function
  // Do not use _hardware_info directry since this data is located in UEFI-app space
  hardware_info = *_hardware_info;
  init_segmentation();
  // To here - Put this part at the top of start() function

  init_frame_buffer(&hardware_info.fb);
  puts("hello world!!\nI'm Jumpei!!!\rThis is a pen.Are you a pen ?\napplepen pine-apple pen\n");

  puts("100 is ");
  puth(100,4);
  puts("\n");


  init_acpi_pm_timer(hardware_info.rsdp);

  puts("wait 2 sec\n");

  pm_timer_wait_millisec(2000);

  puts("finished\n");

  unsigned int freq = measure_lapic_freq_khz();
  puts("freq is ");
  puth(freq,8);
  puts("\n");
  // Do not delete it!
  while (1);
}
