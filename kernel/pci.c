#include "util.h"


union PciConfAddress {
    unsigned int raw;
    struct {
        unsigned int reg_offset : 8;
        unsigned int func_num : 3;
        unsigned int device_num : 5;
        unsigned int bus_num : 8;
        unsigned int reserved : 7;
        unsigned int enable : 1;
    }__attribute((packed));
};

unsigned short conf_addr = 0xcf8;
unsigned short conf_data = 0xcfc;

unsigned int port_in(unsigned short arg1);
void port_out(unsigned short arg1, unsigned int arg2);

static unsigned int read_pci_conf(unsigned int bus, unsigned int device,unsigned int func,unsigned int offset) {
	union PciConfAddress pca;
	pca.reg_offset = offset;
	pca.func_num = func;
	pca.device_num = device;
	pca.bus_num = bus;
	pca.reserved = 0;
	pca.enable = 1;
	port_out(conf_addr,pca.raw);
	unsigned int r_data = port_in(conf_data);
	return r_data;
}

static void write_pci_conf(unsigned int bus, unsigned int device, unsigned int func,unsigned int offset, unsigned int data) {
	union PciConfAddress pca;
	pca.reg_offset = offset;
	pca.func_num = func;
	pca.device_num = device;
	pca.bus_num = bus;
	pca.reserved = 0;
	pca.enable = 1;
	port_out(conf_addr,pca.raw);
	port_out(conf_data,data);
	return;
}

static unsigned int get_nic_pci_header_type() {
	unsigned int reg03 = read_pci_conf(0,3,0,0x0c);
	unsigned int header_type = (reg03 >> 16) & 0b11111111;
	return header_type;
}

void init_nic_pci() {
	unsigned header_type = get_nic_pci_header_type();
	if(header_type != 0) {
		puts("header_type is not 0");
		return;
	}

	unsigned int tenth_bit = 0b10000000000;
	unsigned int reg01_data = read_pci_conf(0,3,0,0x04);
	reg01_data = (reg01_data % (1<<16)) | tenth_bit;
	if((reg01_data & 0b110) != 0b110) {
		puts("bad data\n");
	}
	write_pci_conf(0,3,0,0x04,reg01_data);
	return;
}

unsigned int get_nic_base_address() {
	unsigned int bar;
	unsigned int offset;
	unsigned base_addr;

	for(unsigned int i = 0;i < 6;i++) {
		offset = 0x10 + i * 0x04;
		bar = read_pci_conf(0,3,0,offset);
		if(bar != 0 && (bar & 1) == 0 && ((bar >> 1) & 0b11) == 0x00) {
			base_addr = bar;
			break;
		}
	}

	if(base_addr == 0) {
		puts("32bit physi addr not found");
	}

	return base_addr;
}