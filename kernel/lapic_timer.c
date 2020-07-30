#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"

volatile unsigned int *lvt_timer = (unsigned int *)0xfee00320;
volatile unsigned int *initial_count = (unsigned int *)0xfee00380;
volatile unsigned int *current_count = (unsigned int *)0xfee00390;
volatile unsigned int *divide_config = (unsigned int *)0xfee003e0;
unsigned int lapic_timer_freq_khz;

unsigned int measure_lapic_freq_khz() {
	*lvt_timer = 0;
	*initial_count = 4294967295;
	pm_timer_wait_millisec(1000);
	// puth(*current_count,10);
	lapic_timer_freq_khz = ((*initial_count-*current_count)/1)/1000;
	// puth(*current_count,8);
	// puts("\n");
	return lapic_timer_freq_khz;
}