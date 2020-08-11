#ifndef __SCHED_H__
#define __SCHED_H__

void init_tasks();

void schedule(unsigned long long sp);

#define TASK_NUM 3

extern unsigned long long task_cr3s[TASK_NUM];
extern unsigned long long kernel_cr3;

#endif