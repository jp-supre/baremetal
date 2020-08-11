#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"
#include "interruption.h"
#include "sched.h"
#include "syscall.h"

unsigned long long syscall_puts(char *str){
	puts(str);
	puts("\n");
	return 1;
}

unsigned long long syscall_handler_internal(SYSCALL syscall_id, unsigned long long arg1,
	unsigned long long arg2, unsigned long long arg3) {

	unsigned int ret;

	if (syscall_id == SYSCALL_PUTS)
	{
		char *str = (char *)arg1;
		ret = syscall_puts(str);
	} else {
		ret = 0;
	}

	return ret;
}

