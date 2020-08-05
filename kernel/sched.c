#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"
#include "interruption.h"
#include "sched.h"

#define TASK_NUM 3
// #define STACK_SIZE 4096

// char stack0[STACK_SIZE];
// char stack1[STACK_SIZE];
// char stack2[STACK_SIZE];

// static void task0() {
// 	while(1) {
// 		puts("hello from task0\n");
// 		volatile int i = 100000000;
// 		while(i--);
// 	}
// }

// static void task1() {
// 	while(1) {
// 		puts("hello from task1\n");
// 		volatile int i = 100000000;
// 		while(i--);
// 	}
// }

// static void task2() {
// 	while(1) {
// 		puts("hello from task2\n");
// 		volatile int i = 100000000;
// 		while(i--);
// 	}
// }

struct Task{
	unsigned long long sp;
};

struct Task tasks[TASK_NUM];
unsigned int current_task = 0;

// static void init_task(int idx,unsigned char *stack_bottom,unsigned long long rip) {
// 	unsigned long long *sp = (unsigned long long *)stack_bottom;
// 	unsigned long long ss;
// 	asm volatile ("mov %%ss, %0":"=r"(ss));

// 	unsigned long long rsp = (unsigned long long)stack_bottom;

// 	*(sp-1) = ss;
// 	*(sp-2) = rsp;

// 	unsigned long long current_sp = rsp - 16;

// 	unsigned long long reg64;
//     asm volatile (
//         "mov %%rsp, %0\n"
//         "mov %1, %%rsp\n"
//         "pushfq\n":"=r"(reg64):"m"(current_sp)
//     );
//     asm volatile ("mov %0, %%rsp"::"m"(reg64));

// 	unsigned short cs_pres;
// 	asm volatile ("mov %%cs, %0":"=r"(cs_pres));
// 	unsigned long long cs = cs_pres;

//     *(sp-4) = cs;
//     *(sp-5) = rip;

//     tasks[idx].sp = (unsigned long long)stack_bottom - 160;

//     return;
// }

static void init_task(int idx,unsigned long long app_bottom,unsigned long long app_top) {
	unsigned long long *sp = (unsigned long long *)app_bottom;
	unsigned long long ss;
	asm volatile ("mov %%ss, %0":"=r"(ss));

	unsigned long long rsp = app_bottom;

	*(sp-1) = ss;
	*(sp-2) = rsp;

	unsigned long long current_sp = rsp - 16;

	unsigned long long reg64;
    asm volatile (
        "mov %%rsp, %0\n"
        "mov %1, %%rsp\n"
        "pushfq\n":"=r"(reg64):"m"(current_sp)
    );
    asm volatile ("mov %0, %%rsp"::"m"(reg64));

	unsigned short cs_pres;
	asm volatile ("mov %%cs, %0":"=r"(cs_pres));
	unsigned long long cs = cs_pres;

    *(sp-4) = cs;
    *(sp-5) = app_top;

    tasks[idx].sp = (unsigned long long)app_bottom - 160;

    return;
}



void init_tasks() {
	init_task(1,(unsigned long long)0x106000000,(unsigned long long)0x105000000);
	init_task(2,(unsigned long long)0x107000000,(unsigned long long)0x106000000);

	unsigned long long sp0 = (unsigned long long)0x105000000;
	asm volatile ("mov %0, %%rsp"::"m"(sp0));
	
	unsigned long long rip = (unsigned long long)0x104000000;
	asm volatile("jmp *%0"::"m"(rip));
	return;
}

void schedule(unsigned long long sp) {
	unsigned char now = current_task;
	current_task = (current_task + 1) % 3;
	tasks[now].sp = sp;
	unsigned long long cur_sp = tasks[current_task].sp;

    lapic_set_eoi();

	asm volatile ("mov %0, %%rsp"::"m"(cur_sp));

	asm volatile (
        "pop %r15\n"
        "pop %r14\n"
        "pop %r13\n"
        "pop %r12\n"
        "pop %r11\n"
        "pop %r10\n"
        "pop %r9\n"
        "pop %r8\n"
        "pop %rdi\n"
        "pop %rsi\n"
        "pop %rbp\n"
        "pop %rbx\n"
        "pop %rdx\n"
        "pop %rcx\n"
        "pop %rax\n"
        "iretq\n");

	return;
}



