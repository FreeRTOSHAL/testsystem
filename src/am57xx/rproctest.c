#include <stddef.h>
#include <remoteproc.h>
#include <devs.h>
#define SZ_1M (1 * 1024 * 1024)
#define SZ_2M (2 * 1024 * 1024)
#define SZ_4M (4 * 1024 * 1024)
#define SZ_8M (8 * 1024 * 1024)
#define SZ_16M (16 * 1024 * 1024)
#define SZ_32M (32 * 1024 * 1024)
#define SZ_128M (128 * 1024 * 1024)
#define SZ_256M (256 * 1024 * 1024)

/* IPU Memory Map */
#define L4_DRA7XX_BASE          0x4A000000

/* L4_CFG & L4_WKUP */
#define L4_PERIPHERAL_L4CFG     (L4_DRA7XX_BASE)
#define IPU_PERIPHERAL_L4CFG    0x6A000000

#define L4_PERIPHERAL_L4PER1    0x48000000
#define IPU_PERIPHERAL_L4PER1   0x68000000

#define L4_PERIPHERAL_L4PER2    0x48400000
#define IPU_PERIPHERAL_L4PER2   0x68400000

#define L4_PERIPHERAL_L4PER3    0x48800000
#define IPU_PERIPHERAL_L4PER3   0x68800000

#define L4_PERIPHERAL_L4EMU     0x54000000
#define IPU_PERIPHERAL_L4EMU    0x74000000

#define L3_PERIPHERAL_DMM       0x4E000000
#define IPU_PERIPHERAL_DMM      0x6E000000

#define L3_IVAHD_CONFIG         0x5A000000
#define IPU_IVAHD_CONFIG        0x7A000000

#define L3_IVAHD_SL2            0x5B000000
#define IPU_IVAHD_SL2           0x7B000000

#define L3_TILER_MODE_0_1       0x60000000
#define IPU_TILER_MODE_0_1      0xA0000000

#define L3_TILER_MODE_2         0x70000000
#define IPU_TILER_MODE_2        0xB0000000

#define L3_TILER_MODE_3         0x78000000
#define IPU_TILER_MODE_3        0xB8000000

#define IPU_MEM_TEXT            0x0
#define IPU_MEM_DATA            0x80000000

#define IPU_MEM_IPC_DATA        0x9F000000
#define IPU_MEM_IPC_VRING       0x60000000
#define IPU_MEM_RPMSG_VRING0    0x60000000
#define IPU_MEM_RPMSG_VRING1    0x60004000
#define IPU_MEM_VRING_BUFS0     0x60040000
#define IPU_MEM_VRING_BUFS1     0x60080000

#define IPU_MEM_IPC_VRING_SIZE  SZ_1M
#define IPU_MEM_IPC_DATA_SIZE   SZ_1M

#define IPU_MEM_TEXT_SIZE       (SZ_1M * 6)

/*
 * IPU_MEM_DATA_SIZE contains the size of EXT_DATA + EXT_HEAP
 * defined in the dce_ipu.cfg
 */
#define IPU_MEM_DATA_SIZE       (SZ_1M * 15)

/*
 * Assign fixed RAM addresses to facilitate a fixed MMU table.
 * PHYS_MEM_IPC_VRING & PHYS_MEM_IPC_DATA MUST be together.
 */
/* See CMA BASE addresses in Linux side (for 3.8 & 3.12 kernels):
       arch/arm/mach-omap2/remoteproc.c */
/* For 3.14 kernels version and above, look for CMA reserved memory node in
       the board dts file. */
#define PHYS_MEM_IPC_VRING      0x95800000

/*
 * Sizes of the virtqueues (expressed in number of buffers supported,
 * and must be power of 2)
 */
#define IPU_RPMSG_VQ0_SIZE      256
#define IPU_RPMSG_VQ1_SIZE      256

/* flip up bits whose indices represent features we support */
#define RPMSG_IPU_C0_FEATURES   1
#define RESOURCE_TABLE SECTION(".resource_table") USED NO_REORDER
struct rtable {
	struct resource_table resource_table;
	uintptr_t offset[14];
	struct fw_rsc_carveout text_cout;
	struct fw_rsc_carveout data_cout;
	struct fw_rsc_carveout ipcdata_cout;
	struct fw_rsc_devmem devmem1;
	struct fw_rsc_devmem devmem2;
	struct fw_rsc_devmem devmem3;
	struct fw_rsc_devmem devmem4;
	struct fw_rsc_devmem devmem5;
	struct fw_rsc_devmem devmem6;
	struct fw_rsc_devmem devmem7;
	struct fw_rsc_devmem devmem8;
	struct fw_rsc_devmem devmem9;
	struct fw_rsc_devmem devmem10;
	struct fw_rsc_devmem devmem11;
	struct fw_rsc_vdev rpmsg_vdev;
	struct fw_rsc_vdev_vring rpmsg_vring0;
	struct fw_rsc_vdev_vring rpmsg_vring1;
} PACKED;
struct rtable RESOURCE_TABLE rtable = {
	.resource_table = {
		.ver = 1,
		.num = 14,
	},
	.offset = {
		offsetof(struct rtable, rpmsg_vdev),
		offsetof(struct rtable, text_cout),
		offsetof(struct rtable, data_cout),
		offsetof(struct rtable, ipcdata_cout),
		offsetof(struct rtable, devmem1),
		offsetof(struct rtable, devmem2),
		offsetof(struct rtable, devmem3),
		offsetof(struct rtable, devmem4),
		offsetof(struct rtable, devmem5),
		offsetof(struct rtable, devmem6),
		offsetof(struct rtable, devmem7),
		offsetof(struct rtable, devmem9),
		offsetof(struct rtable, devmem10),
		offsetof(struct rtable, devmem11),
	},
	.rpmsg_vdev = {
		.type = RSC_VDEV,
		.id = 7,
		.notifyid = 0,
		.dfeatures = RPMSG_IPU_C0_FEATURES,
		.gfeatures = 0,
		.config_len = 0,
		.status = 0,
		.num_of_vrings = 2,
	},
	.rpmsg_vring0 = {
		.da = IPU_MEM_RPMSG_VRING0,
		.align = 4096,
		.num = IPU_RPMSG_VQ0_SIZE,
		.notifyid = 1,
	},
	.rpmsg_vring1 = {
		.da = IPU_MEM_RPMSG_VRING1,
		.align = 4096,
		.num = IPU_RPMSG_VQ1_SIZE,
		.notifyid = 2,
	},
	.text_cout = {
		.type = RSC_CARVEOUT,
		.da = IPU_MEM_TEXT,
		.pa = 0x0,
		.len = IPU_MEM_TEXT_SIZE,
		.flags = 0,
		.name = "IPU_MEM_TEXT",
	},
	.data_cout = {
		.type = RSC_CARVEOUT,
		.da = IPU_MEM_DATA,
		.pa = 0x0,
		.len = IPU_MEM_DATA_SIZE,
		.flags = 0,
		.name = "IPU_MEM_DATA",
	},
	.ipcdata_cout = {
		.type = RSC_CARVEOUT,
		.da = IPU_MEM_IPC_DATA,
		.pa = 0x0,
		.len = IPU_MEM_IPC_DATA_SIZE,
		.flags = 0,
		.name = "IPU_MEM_IPC_DATA",
	},
	.devmem1 = {
		.type = RSC_DEVMEM,
		.da = IPU_MEM_IPC_VRING,
		.pa = PHYS_MEM_IPC_VRING,
		.len = IPU_MEM_IPC_VRING_SIZE,
		.flags = 0,
		.name = "IPU_TILER_MODE_0_1"
	},
	.devmem2 = {
		.type = RSC_DEVMEM,
		.da = IPU_TILER_MODE_0_1,
		.pa = L3_TILER_MODE_0_1,
		.len = SZ_256M,
		.flags = 0,
		.name = "IPU_TILER_MODE_0_1"
	},
	.devmem3 = {
		.type = RSC_DEVMEM,
		.da = IPU_TILER_MODE_2,
		.pa = L3_TILER_MODE_2,
		.len = SZ_128M,
		.flags = 0,
		.name = "IPU_TILER_MODE_2"
	},
	.devmem4 = {
		.type = RSC_DEVMEM,
		.da = IPU_TILER_MODE_3,
		.pa = L3_TILER_MODE_3,
		.len = SZ_128M,
		.flags = 0,
		.name = "IPU_TILER_MODE_3"
	},
	.devmem5 = {
		.type = RSC_DEVMEM,
		.da = IPU_PERIPHERAL_L4PER1,
		.pa = L4_PERIPHERAL_L4PER1,
		.len = SZ_2M,
		.flags = 0,
		.name = "IPU_PERIPHERAL_L4PER1",
	},
	.devmem6 = {
		.type = RSC_DEVMEM,
		.da = IPU_PERIPHERAL_L4PER2,
		.pa = L4_PERIPHERAL_L4PER2,
		.len = SZ_4M,
		.flags = 0,
		.name = "IPU_PERIPHERAL_L4PER2"
	},
	.devmem7 = {
		.type = RSC_DEVMEM,
		.da = IPU_PERIPHERAL_L4PER3,
		.pa = L4_PERIPHERAL_L4PER3,
		.len = SZ_8M,
		.flags = 0,
		.name = "IPU_PERIPHERAL_L4PER3",
	},
	.devmem8 = {
		.type = RSC_DEVMEM,
		.da = IPU_PERIPHERAL_L4EMU,
		.pa = L4_PERIPHERAL_L4EMU,
		.len = SZ_16M,
		.flags = 0,
		.name = "IPU_PERIPHERAL_L4EMU",
	},
	.devmem9 = {
		.type = RSC_DEVMEM,
		.da = IPU_IVAHD_CONFIG,
		.pa = L3_IVAHD_CONFIG,
		.len = SZ_16M,
		.flags = 0,
		.name = "IPU_IVAHD_CONFIG"
	},
	.devmem10 = {
		.type = RSC_DEVMEM,
		.da = IPU_IVAHD_SL2,
		.pa = L3_IVAHD_SL2,
		.len = SZ_16M,
		.flags = 0,
		.name = "IPU_IVAHD_SL2",
	},
	.devmem11 = {
		.type = RSC_DEVMEM,
		.da = IPU_PERIPHERAL_DMM,
		.pa = L3_PERIPHERAL_DMM,
		.len = SZ_1M,
		.flags = 0,
		.name = "IPU_PERIPHERAL_DMM"
	},
};
/*void rprocTest_init() {
	struct rproc *rproc;
	struct mailbox *mbox = mailbox_init(MAILBOX0_ID);
	CONFIG_ASSERT(mbox != NULL);
	rproc = rproc_init(&rprocMailbox_ops, mbox, (struct resource_table *) &rtable, 1, false);
	CONFIG_ASSERT(rproc != NULL);
}*/
