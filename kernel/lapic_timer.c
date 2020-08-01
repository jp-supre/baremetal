#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"
#include "interruption.h"
#include "sched.h"

volatile unsigned int *lvt_timer = (unsigned int *)0xfee00320;
volatile unsigned int *initial_count = (unsigned int *)0xfee00380;
volatile unsigned int *current_count = (unsigned int *)0xfee00390;
volatile unsigned int *divide_config = (unsigned int *)0xfee003e0;
unsigned int lapic_timer_freq_khz;

unsigned int measure_lapic_freq_khz() {
	*lvt_timer = 0;
	*initial_count = 4294967295;
	*divide_config |= 0b00000000000000000000000000001011;
	pm_timer_wait_millisec(1000);
	// puth(*current_count,10);
	lapic_timer_freq_khz = ((*initial_count-*current_count)/1)/1000;
	// puth(*current_count,8);
	// puts("\n");
	return lapic_timer_freq_khz;
}


volatile unsigned int *lapic_eoi = (unsigned int *)0xfee000b0;

void (*reserved_callback)(unsigned long long);

void lapic_periodic_exec(unsigned int msec, void *callback) {
	init_intr();
	unsigned int lvt_timer_mask = 0b00000000000000100000000000100000;
	*lvt_timer = lvt_timer_mask;
	reserved_callback = callback;
	unsigned int wait_clock = msec * lapic_timer_freq_khz;
	// puth(wait_clock,10);
	*initial_count = wait_clock;
	return;
}


void lapic_intr_handler_internal(unsigned long long arg1) {
	reserved_callback(arg1);
	return;
}

void lapic_set_eoi() {
	*lapic_eoi = 1;
	return;
}