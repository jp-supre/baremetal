#include "hardware.h"
#include "segmentation.h"
#include "util.h"
#include "pm_timer.h"
#include "lapic_timer.h"
#include "interruption.h"

struct InterruptDescriptor{
	unsigned short offset_lo;
	unsigned short segment;
	unsigned short attribute;
	unsigned short offset_mid;
	unsigned int offset_hi;
	unsigned int reserved;
}__attribute__((packed));

struct InterruptDescriptor IDT[256];

unsigned char data[10];

static void load_Idt_to_Idtr() {
	unsigned long long int idt_addr = (unsigned long long int)IDT;
	data[0] = 255;
	data[1] = 15;
	data[2] = idt_addr & 255;
	data[3] = (idt_addr >> 8) & 255;
	data[4] = (idt_addr >> 16) & 255;
	data[5] = (idt_addr >> 24) & 255;
	data[6] = (idt_addr >> 32) & 255;
	data[7] = (idt_addr >> 40) & 255;
	data[8] = (idt_addr >> 48) & 255;
	data[9] = (idt_addr >> 56) & 255;

	asm volatile ("lidt %0" :: "m"(data));
	return;
}

static void register_intr_handler(unsigned char index,unsigned long long offset,
 unsigned short segment, unsigned short attribute) {
	unsigned short mask_16 = 0b1111111111111111;
	struct InterruptDescriptor id;

	id.offset_lo = offset & mask_16;
	id.segment = segment;
	id.attribute = attribute;
	id.offset_mid = (offset >> 16) & mask_16;
	id.offset_hi = offset >> 32;

	IDT[index] = id;
	return;
}

void init_intr() {
	load_Idt_to_Idtr();

	// unsigned long long handler;
 //    asm volatile ("lea lapic_intr_handler(%%rip), %[handler]" : [handler]"=r"(handler));

    unsigned short segment;
    asm volatile ("mov %%cs, %0" : "=r"(segment));

    unsigned short attribute = 0b1000111000000000;

    // register_intr_handler(32,handler,segment,attribute);

    unsigned long long handler2;
    asm volatile ("lea syscall_handler(%%rip), %[handler2]" : [handler2]"=r"(handler2));

    register_intr_handler(128,handler2,segment,attribute);

    asm volatile ("sti");

    return;

}