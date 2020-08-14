#define TX_DESCRIPTORS_NUM 8
#define RX_DESCRIPTORS_NUM 8
#define RX_FRAME_BUFFER_SIZE 2048

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

struct RxDescriptor {
    unsigned long long buffer_addr;
    unsigned int length : 16;
    unsigned int checksum : 16;
    unsigned int sta : 8;
    unsigned int errors : 8;
    unsigned int special : 16;
}__attribute__((packed));


static struct TxDescriptor tx_descriptors[TX_DESCRIPTORS_NUM]__attribute__((aligned(16)));
static struct RxDescriptor rx_descriptors[RX_DESCRIPTORS_NUM]__attribute__((aligned(16)));

static unsigned char rx_frame_buffers[RX_DESCRIPTORS_NUM][RX_FRAME_BUFFER_SIZE];

static unsigned int nic_base_address;

static unsigned int tx_current_idx;
static unsigned int rx_current_idx;



static void set_nic_register(unsigned short offset, unsigned int value) {
    unsigned int *addr = nic_base_address + offset;
    *addr = value;
    return;
}

static void init_tx() {
    struct TxDescriptor tx;
    tx.buffer_addr = 0;
    tx.special = 0;
    tx.css = 0;
    tx.reserved = 0;
    tx.sta = 0;
    tx.cmd = 0b1001;
    tx.cso = 0;
    tx.length = 0;

    for (unsigned int i=0; i < TX_DESCRIPTORS_NUM; i++) {
        tx_descriptors[i] = tx; 
    }

    tx_current_idx = 0;

    unsigned int high_half = (unsigned int)((unsigned long long)tx_descriptors >> 32); //ring bufferの先頭アドレス上位32ビット
    unsigned int low_half = (unsigned int)((unsigned long long)tx_descriptors & 0xffffffff); //ring bufferの先頭アドレス下位32ビット
    
    set_nic_register(0x3800, low_half);
    set_nic_register(0x3804, high_half);

    unsigned int leng = 16 * TX_DESCRIPTORS_NUM;
    set_nic_register(0x3808, leng);

    set_nic_register(0x3810, 0);
    set_nic_register(0x3818, 0);

    unsigned int tctl_config = 0b1010 << 22 | 0x40 << 12 | 0x0f << 4 | 0b1010;
    set_nic_register(0x400, tctl_config);


    return;
}

static void init_rx() {
    for (unsigned int i = 0; i < RX_DESCRIPTORS_NUM; i++) {
        rx_descriptors[i].buffer_addr = rx_frame_buffers[i];
        rx_descriptors[i].errors = 0;
        rx_descriptors[i].sta = 0;
    }

    rx_current_idx = 0;

    unsigned int high_half = (unsigned int)((unsigned long long)rx_descriptors >> 32);
    unsigned int low_half = (unsigned int)((unsigned long long)rx_descriptors & 0xffffffff);
    
    set_nic_register(0x2800, low_half);
    set_nic_register(0x2804, high_half);

    unsigned int ring_buf_len = 16 * TX_DESCRIPTORS_NUM;

    set_nic_register(0x2810, 0);
    set_nic_register(0x2818, RX_DESCRIPTORS_NUM-1);

    unsigned int config = 0b00 << 16 | 0b1 << 15 | 0b11110;
    set_nic_register(0x100, config);

    return;
}

void init_nic(unsigned int nic_address) {
    nic_base_address = nic_address;

    set_nic_register(0x00d8, 0b11111111111111111);

    init_tx();
    init_rx();
    
    return;
} 

unsigned char send_frame(void *buffer, unsigned short len) {
    tx_descriptors[tx_current_idx].buffer_addr = (unsigned long long)buffer;
    tx_descriptors[tx_current_idx].length = (unsigned int)len;

    unsigned int tx_next_idx = (tx_current_idx + 1) % TX_DESCRIPTORS_NUM;

    set_nic_register(0x3818, tx_next_idx);

    while(tx_descriptors[tx_current_idx].sta == 0);

    tx_descriptors[tx_current_idx].sta = 0;

    tx_current_idx = (tx_current_idx + 1) % TX_DESCRIPTORS_NUM;

    return 0;
}

unsigned short receive_frame(void *buffer) {

    if (rx_descriptors[rx_current_idx].sta != 0) {

        unsigned int len = rx_descriptors[rx_current_idx].length;
        unsigned char *point;

        for (unsigned int i = 0; i < len; i++) {
            point = (unsigned char *)buffer + i;
            *point = rx_frame_buffers[rx_current_idx][i];
        }

        tx_descriptors[rx_current_idx].sta = 0;

        set_nic_register(0x2818, rx_current_idx);

        rx_current_idx = (rx_current_idx + 1) % TX_DESCRIPTORS_NUM;


        return len;
    }
    
    return 0;
}