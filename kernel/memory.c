#include "memory.h"
#include "util.h"

#define TASK_NUM 3

unsigned long long task_cr3s[TASK_NUM];
unsigned long long kernel_cr3;

struct Entry {
	unsigned int present : 1;
	unsigned int writabel : 1;
	unsigned int user_accessible : 1;
	unsigned int write_through_caching :1;
	unsigned int disable_cache :1;
	unsigned int accessed :1;
	unsigned int dirty: 1;
	unsigned int huge_page :1;
	unsigned int global :1;
	unsigned int available1 :3;
	unsigned long long physical_addr :40;
	unsigned int available2 :11;
	unsigned int no_execute :1;
}__attribute__((packed));

// unsigned long long task_cr3s[TASK_NUM];
// unsigned long long kernel_cr3;

struct Entry PML4s[TASK_NUM][512]__attribute__((aligned(4096)));
struct Entry PDPs[TASK_NUM][512]__attribute__((aligned(4096)));
struct Entry PDs[TASK_NUM][512]__attribute__((aligned(4096)));
struct Entry PTs[TASK_NUM][8][512]__attribute__((aligned(4096)));

struct Entry kernel_PD[512]__attribute__((aligned(4096)));
struct Entry kernel_PTs[8][512]__attribute__((aligned(4096)));

struct Entry io_PD[512]__attribute__((aligned(4096)));
struct Entry fb_PT[512]__attribute__((aligned(4096)));
struct Entry lapic_PT[512]__attribute__((aligned(4096)));


unsigned long long mask = 0b111111111;

unsigned int fst_9bit(unsigned long long addr) {
	addr = (addr >> 39);
	addr = (unsigned int)(addr & mask);
	return addr;
}

unsigned int snd_9bit(unsigned long long addr) {
	addr = (addr >> 30);
	addr = (unsigned int)(addr & mask);
	return addr;
}

unsigned int trd_9bit(unsigned long long addr) {
	addr = (addr >> 21);
	addr = (unsigned int)(addr & mask);
	return addr;
}

unsigned int forth_9bit(unsigned long long addr) {
	addr = (addr >> 12);
	addr = (unsigned int)(addr & mask);
	return addr;
}

unsigned int last_12bit(unsigned long long addr) {
	unsigned long long mk = 0b111111111111;
	addr = (unsigned int)(addr & mk);
	return addr;
}


void init_virtual_memory() {
	task_cr3s[0] = (unsigned long long)PML4s[0];
	task_cr3s[1] = (unsigned long long)PML4s[1];
	task_cr3s[2] = (unsigned long long)PML4s[2];

	unsigned long long cr3;
	asm volatile ("mov %%cr3, %0":"=r"(cr3));
	kernel_cr3 = cr3;

	for(unsigned int i = 0;i < TASK_NUM; i++) {
		for (unsigned int j = 0; j < 512; j++) {
			PML4s[i][j].present = PML4s[i][j].writabel = 1;
			PDPs[i][j].present = PDPs[i][j].writabel = 1;
			PDs[i][j].present = PDs[i][j].writabel = 1;
			for(unsigned int k = 0;k < 8;k++) {
				PTs[i][k][j].present = PTs[i][k][j].writabel = 1;
				if(i==0) kernel_PTs[k][j].present = kernel_PTs[k][j].writabel = 1;
			}
			if(i==0) kernel_PD[j].present = kernel_PD[j].writabel = 1;
			if(i==0) io_PD[j].present = io_PD[j].writabel = 1;
			if(i==0) fb_PT[j].present = fb_PT[j].writabel = 1;
			if(i==0) lapic_PT[j].present = lapic_PT[j].writabel = 1;
		}
	}


	for (unsigned int i = 0;i < 0x1000;i++) {
		unsigned long long virtual_addr = 0x040000000 + 0x1000 * i;
		unsigned long long physi_addr = 0x104000000 + 0x1000 * i;
		PML4s[0][fst_9bit(virtual_addr)].physical_addr = (unsigned long long)PDPs[0] >> 12;
		PDPs[0][snd_9bit(virtual_addr)].physical_addr = (unsigned long long)PDs[0] >> 12;
		PDs[0][trd_9bit(virtual_addr)].physical_addr = (unsigned long long)PTs[0][trd_9bit(virtual_addr)] >> 12;
		PTs[0][trd_9bit(virtual_addr)][forth_9bit(virtual_addr)].physical_addr = physi_addr >> 12;
	}

	for (unsigned int i = 0;i < 0x1000;i++) {
		unsigned long long virtual_addr = 0x040000000 + 0x1000 * i;
		unsigned long long physi_addr = 0x105000000 + 0x1000 * i;
		PML4s[1][fst_9bit(virtual_addr)].physical_addr = (unsigned long long)PDPs[1] >> 12;
		PDPs[1][snd_9bit(virtual_addr)].physical_addr = (unsigned long long)PDs[1] >> 12;
		PDs[1][trd_9bit(virtual_addr)].physical_addr = (unsigned long long)PTs[1][trd_9bit(virtual_addr)] >> 12;
		PTs[1][trd_9bit(virtual_addr)][forth_9bit(virtual_addr)].physical_addr = physi_addr >>12;
	}

	for (unsigned int i = 0;i < 0x1000;i++) {
		unsigned long long virtual_addr = 0x040000000 + 0x1000 * i;
		unsigned long long physi_addr = 0x106000000 + 0x1000 * i;
		PML4s[2][fst_9bit(virtual_addr)].physical_addr = (unsigned long long)PDPs[2] >> 12;
		PDPs[2][snd_9bit(virtual_addr)].physical_addr = (unsigned long long)PDs[2] >> 12;
		PDs[2][trd_9bit(virtual_addr)].physical_addr = (unsigned long long)PTs[2][trd_9bit(virtual_addr)] >> 12;
		// PDs[2][trd_9bit(virtual_addr)].physical_addr = (unsigned long long)PTs[2][trd_9bit(virtual_addr)] >> 12;
		PTs[2][trd_9bit(virtual_addr)][forth_9bit(virtual_addr)].physical_addr = physi_addr >> 12;
		// PTs[2][trd_9bit(virtual_addr)][forth_9bit(virtual_addr)].physical_addr = physi_addr >> 12;
	}

	for (unsigned int i = 0;i<TASK_NUM;i++) {
		unsigned virtual_addr = 0x0fee00000;
		unsigned physi_addr = 0x0fee00000;
		PML4s[i][fst_9bit(virtual_addr)].physical_addr = (unsigned long long)PDPs[i] >> 12;
		PDPs[i][snd_9bit(virtual_addr)].physical_addr = (unsigned long long)io_PD >> 12;
		io_PD[trd_9bit(virtual_addr)].physical_addr = (unsigned long long)lapic_PT >>12;
		for(unsigned int j = 0;j<512;j++) {
			// lapic_PT[forth_9bit(virtual_addr)].physical_addr = physi_addr >> 12;
			lapic_PT[j].physical_addr = physi_addr >> 12;
		}
	}

	for (unsigned int i = 0; i < 512; i++) {
		for (unsigned int j = 0; j < TASK_NUM;j++) {
			unsigned long long virtual_addr = 0x0c0000000 + 0x1000 * i;
			unsigned long long physi_addr = 0x0c0000000 + 0x1000 * i;
			PML4s[j][fst_9bit(virtual_addr)].physical_addr = (unsigned long long)PDPs[j] >> 12;
		    PDPs[j][snd_9bit(virtual_addr)].physical_addr = (unsigned long long)io_PD >> 12;
		    io_PD[trd_9bit(virtual_addr)].physical_addr = (unsigned long long)fb_PT >> 12;
		    fb_PT[forth_9bit(virtual_addr)].physical_addr = physi_addr >> 12;
		}
	}

	for (unsigned int i = 0; i < 0x1000; i++) {
		for (unsigned int j = 0; j < TASK_NUM;j++) {
			unsigned long long virtual_addr = 0x100000000 + 0x1000 * i;
			unsigned long long physi_addr = 0x100000000 + 0x1000 * i;
			PML4s[j][fst_9bit(virtual_addr)].physical_addr = (unsigned long long)PDPs[j] >> 12;
		    PDPs[j][snd_9bit(virtual_addr)].physical_addr = (unsigned long long)kernel_PD >> 12;
		    kernel_PD[trd_9bit(virtual_addr)].physical_addr = (unsigned long long)kernel_PTs[trd_9bit(virtual_addr)] >> 12;
		    kernel_PTs[trd_9bit(virtual_addr)][forth_9bit(virtual_addr)].physical_addr = physi_addr >> 12;
		}
	}
}


// #include "sched.h"
// #include "util.h"

// struct Entry {
//     unsigned long long entry;
// }__attribute__((packed));

// unsigned long long task_cr3s[TASK_NUM];

// unsigned long long get_task_cr3s(unsigned int idx) { //カプセル化
//     return task_cr3s[idx];
// }


// unsigned long long kernel_cr3;

// unsigned long long get_kernel_cr3() { //カプセル化
//     return kernel_cr3;
// }

// struct Entry PML4s[TASK_NUM][512]__attribute__((aligned(4096)));
// struct Entry PDPs[TASK_NUM][512]__attribute__((aligned(4096)));
// struct Entry PDs[TASK_NUM][512]__attribute__((aligned(4096)));
// struct Entry PTs[TASK_NUM][8][512]__attribute__((aligned(4096)));


// struct Entry kernel_PD[512]__attribute__((aligned(4096)));
// struct Entry kernel_PTs[8][512]__attribute__((aligned(4096)));

// struct Entry io_PD[512]__attribute__((aligned(4096)));
// struct Entry fb_PT[512]__attribute__((aligned(4096)));
// struct Entry lapic_PT[512]__attribute__((aligned(4096)));

// //格納してほしいアドレスを渡すと、エントリにして返す関数
// unsigned long long create_entry(unsigned long long physical) {
//     unsigned long long bottom_12bits = 0b11;//１ページテーブルエントリ下位12ビットの値を設定。writable, presentビットだけ立てておいた
//     return physical + bottom_12bits; //上位12ビットは0, 下位12ビットは上の通り、中40ビットはアドレスの下位12ビットを切り落としたものになっている。アドレスはすべてたかだか52bit長であり、また、ページテーブルは4KiBアラインメントされているので、下位12ビットと上位12ビットはすべて0だという保証があるからこれで問題ないはず
// }

// void init_virtual_memory() {

//     //アドレスのマッピングを行う
//     unsigned int i; //appsの番号に対応
//     unsigned int j; //P2テーブルのインデックスに対応
//     unsigned int k; //P1テーブルのインデックスに対応

//     for (i = 0; i < TASK_NUM; i++) {
//         task_cr3s[i] = (unsigned long long)PML4s[i];//task_cr3sに各タスクのPML4テーブルの先頭を格納
//         puts("memory\n");
//         puth(task_cr3s[i],10);
//         puts("\n");
//         PML4s[i][0].entry = create_entry((unsigned long long)PDPs[i]); //appsもfbもlapicもkernelも、仮想アドレス48ビット中上位9bitは0なので、PML4s[_][0]を参照する。よってここにPDPsの先頭アドレスをもったエントリを入れる。
//         PDPs[i][1].entry = create_entry((unsigned long long)PDs[i]); //appsに割り当てられる仮想アドレスの上位10~18ビットは1なので、PDsの１番目のエントリを見ることになる。よってここにPDsの先頭アドレスを格納
//         PDPs[i][3].entry = create_entry((unsigned long long)io_PD); //fbもlapicも仮想アドレスの上位10~18ビットが3なので。
//         PDPs[i][4].entry = create_entry((unsigned long long)kernel_PD); //kernelの仮想アドレスの上位10~18ビットが4なので。

//         //まずはappsのページテーブルを埋めていく。
//         for (j = 0; j < 8; j++) {
//             PDs[i][j].entry = create_entry((unsigned long long)PTs[i][j]); //appsに割り当てられる仮想アドレスの上位19~27ビットは0~7を取りうる(スタックの底がちょうど8)ので、0の場合から7の場合までで計8通りのP1テーブルに繋がなければならない
//             for (k = 0; k < 512; k++) {
//                 PTs[i][j][k].entry = create_entry((j << 21) + (k << 12) + 0x104000000 + i*0x1000000); //iがappの番号-1になっており、app1は0x104000000~0x105000000, app2は0x105000000~...のようになっているので、iの値によってベースとなる物理アドレスを変えているのが後半。前半のシフト演算は、仮想アドレスと物理アドレスの下位24ビットが変わらないことから、下位23~21ビットがjであり、下位20~12ビットがkであることを利用.
//             }
//         }//完了

//         //次はfbとlapicで分かれる。
//         io_PD[0].entry = create_entry((unsigned long long)fb_PT); //fbの仮想アドレスの上位19~27ビットは0しか取らないので。
//         io_PD[503].entry = create_entry((unsigned long long)lapic_PT); //lapicの仮想アドレスの上位19~27ビットは503しか取らないので。

//         //fbから埋めていく
//         for (k = 0; k < 512; k++) {
//             fb_PT[k].entry = create_entry((3 << 30) + (0 << 21) + (k << 12)); //fbは仮想アドレスと物理アドレスが一致するので、仮想アドレスの下位12ビット以外をそのまま入れれば良い
//         }//完了

//         //lapicを埋める
//         for (k = 0; k < 512; k++) {
//             lapic_PT[k].entry = create_entry((3 << 30) + (503 << 21) + (k << 12)); //lapicも仮想アドレスと物理アドレスが一致する上、下位12ビットより上位のビットは空間内で変わらないので1エントリでよい。
//         }
//         //完了

//         //kernelを埋めていく
//         for (j = 0; j < 8; j++) {
//             kernel_PD[j].entry = create_entry((unsigned long long)kernel_PTs[j]);
//             for (k = 0; k < 512; k++) {
//                 kernel_PTs[j][k].entry = create_entry((4ull << 30) + (j << 21) + (k << 12)); //kernelも仮想アドレスと物理アドレスが一致する。
//             }
//         }//完了
//     }//アドレスのマッピング完了

//     asm volatile ("mov %%cr3, %0":"=r"(kernel_cr3));//カーネル空間を作る、現在のPML4の先頭アドレスをkernel_cr3に格納

//     return;

// }