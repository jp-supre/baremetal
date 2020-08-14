struct TxDescriptor {
    unsigned long long buffer_addr;
    unsigned int length : 16;
    unsigned int cso : 8;
    unsigned int cmd : 8;
    unsigned int sta : 4;
    unsigned int reserved : 4;
    unsigned int css : 8;
    unsigned int special : 16;
}__attribute__((packed));

static unsigned int nic_base_address;

static struct TxDescriptor tx_descriptors[8]__attribute__((aligned(16)));

static unsigned int tx_current_idx;

static void set_nic_register(unsigned short offset, unsigned int value) {
    unsigned int *addr = nic_base_address + offset;
    *addr = value;
    return;
}

static void init_tx() {
	struct TxDescriptor td;
    td.buffer_addr = 0;
    td.special = 0;
    td.css = 0;
    td.reserved = 0;
    td.sta = 0;
    td.cmd = 0b1001;
    td.cso = 0;
    td.length = 0;

    for (unsigned int i=0; i < 8; i++) {
        tx_descriptors[i] = td; 
    }

    tx_current_idx = 0;

    unsigned int high_half = (unsigned int)((unsigned long long)tx_descriptors >> 32);
    unsigned int low_half = (unsigned int)((unsigned long long)tx_descriptors & 0xffffffff);

    set_nic_register(0x3800, low_half);
    set_nic_register(0x3804, high_half);

    unsigned int ring_buf_len = 16 * 8;
    set_nic_register(0x3808, ring_buf_len);

    set_nic_register(0x3810, 0);
    set_nic_register(0x3818, 0);

    unsigned int config = 0b1010 << 22 | 0x40 << 12 | 0x0f << 4 | 0b1010;
    set_nic_register(0x400, config);


    return;
}

void init_nic(unsigned int nic_address) {
    nic_base_address = nic_address;

    set_nic_register(0x00d8, 0b11111111111111111);

    init_tx();

    return;
}

unsigned char send_frame(void *buffer, unsigned short len) {
    tx_descriptors[tx_current_idx].buffer_addr = (unsigned long long)buffer;
    tx_descriptors[tx_current_idx].length = (unsigned int)len;

    tx_current_idx = (tx_current_idx + 1) % 8;

    set_nic_register(0x3818, tx_current_idx);

    while(tx_descriptors[tx_current_idx-1].sta == 0);

    return 0;
}