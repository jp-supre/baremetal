#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"

const unsigned int freq_hz = 3579545;

unsigned short pm_timer_blk;

char pm_timer_is_32;

const char fadt_sig[4] = "FACP";

void init_acpi_pm_timer(struct RSDP *rsdp) {
  unsigned long long xsdt_addr = rsdp->xsdt_address;
  struct SDTH sdth = ((struct XSDT *)xsdt_addr)->sdth;

  int num_of_other_tables = (sdth.length - sizeof(struct SDTH))/sizeof(unsigned long long);

  unsigned long long bottom = xsdt_addr + sizeof(struct SDTH);
  
  for (int i = 0;i < num_of_other_tables;i++) {
    unsigned long long *table_addr = (unsigned long long *)(bottom + (i * sizeof(unsigned long long)));
    char *a = "FACP";
    
    if (strncomp(a,(char *) *table_addr) == 0) {
      pm_timer_blk = *(unsigned short *)(*table_addr+(unsigned long long)76);
      // puth(pm_timer_blk,16);
      if (*(unsigned short *)(*table_addr+(unsigned long long)112) >> 8 & 1) {
        pm_timer_is_32 = 1;
      } else {
         pm_timer_is_32 = 0;  
      }
      break;
    }
  }
}



unsigned long get_tmr(unsigned short tm_blk);

void pm_timer_wait_millisec (unsigned int mesec) {
  unsigned int prev_clock = get_tmr(pm_timer_blk);
  unsigned int wait_clock = freq_hz*(mesec/1000);
  unsigned int max_num_24 = 0x00ffffff;
  unsigned int max_num_32 = 0xffffffff;

  // puts("\n");
  // puth(prev_clock,10);
  // puts("\n");
  // puth(wait_clock,10);
  // puts("\n");

  unsigned int sum=0;
  unsigned int now_clock;

  if (pm_timer_is_32)
  {
    while(1) {
      now_clock = get_tmr(pm_timer_blk);
      if (now_clock < prev_clock)
      {
        sum += (now_clock + (max_num_32 - prev_clock));
      } else {
        sum += (now_clock - prev_clock);
      }
      if (sum >= wait_clock)
      {
        break;
      }
      prev_clock = now_clock;
    }
  } else {
    // puts("24\n");
    while(1) {
      now_clock = get_tmr(pm_timer_blk);
      if (now_clock < prev_clock)
      {
        sum += (now_clock + (max_num_24 - prev_clock));
      } else {
        sum += (now_clock - prev_clock);
      }
      if (sum >= wait_clock)
      {
        // puth(sum,10);
        break;
      }
      prev_clock = now_clock;
      // puth(sum,10);
      // puts("\n");
    }
  }

  // puts("finished\n");
  return;
}