#include "sched.h"
#include "memory.h"
#include "util.h"
#include "lapic_timer.h" 

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
	unsigned long long cr3;
};

struct Task tasks[TASK_NUM];
unsigned int current_task = 0;

extern unsigned long long task_cr3s[TASK_NUM];
extern unsigned long long kernel_cr3;

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
	asm volatile ("mov %0, %%cr3"::"r"(tasks[idx].cr3));
	unsigned long long *sp = (unsigned long long *)app_bottom;
	unsigned long long ss;
	asm volatile ("mov %%ss, %0":"=r"(ss));

	unsigned long long rsp = app_bottom;

	*(sp-1) = ss;
	*(sp-2) = rsp;

	unsigned long long now_sp = rsp - 8*2;

	unsigned long long reg64;
    asm volatile (
        "mov %%rsp, %0\n"
        "mov %1, %%rsp\n"
        "pushfq\n":"=r"(reg64):"m"(now_sp)
    );
    asm volatile ("mov %0, %%rsp"::"m"(reg64));

	unsigned short cs_pres;
	asm volatile ("mov %%cs, %0":"=r"(cs_pres));
	unsigned long long cs = cs_pres;

    *(sp-4) = cs;
    *(sp-5) = app_top;

    tasks[idx].sp = (unsigned long long)app_bottom - 8*20;

    asm volatile ("mov %0, %%cr3"::"r"(kernel_cr3));

    return;
}



void init_tasks() {
	tasks[0].cr3 = task_cr3s[0];
	tasks[1].cr3 = task_cr3s[1];
	tasks[2].cr3 = task_cr3s[2];

	init_task(1,(unsigned long long)0x041000000,(unsigned long long)0x040000000);
	init_task(2,(unsigned long long)0x041000000,(unsigned long long)0x040000000);

	// while(1);

	// puts("sch122\n");

	unsigned long long sp0 = (unsigned long long)0x041000000;
	// asm volatile ("mov %0, %%rsp"::"m"(sp0));

	// puts("sch129\n");
	
	unsigned long long rip = (unsigned long long)0x040000000;

	// while(1);

	// while(1);

	asm volatile ("mov %0, %%cr3"::"r"(tasks[0].cr3));

	// puts("sch133\n");

	asm volatile ("mov %0, %%rsp"::"m"(sp0));

	// while(1);

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

	asm volatile ("mov %0, %%cr3"::"r"(tasks[current_task].cr3));

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



