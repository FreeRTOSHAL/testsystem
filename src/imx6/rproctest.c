#include <stddef.h>
#include <remoteproc.h>
#include <remoteproc_mailbox.h>
#include <mailbox.h>
#include <devs.h>
#define RESOURCE_TABLE SECTION(".resource_table") USED NO_REORDER
struct rtable {
	struct resource_table resource_table;
	uintptr_t offset[3];
	struct fw_rsc_carveout flash;
	struct fw_rsc_vdev console;
	struct fw_rsc_vdev_vring consoleRX;
	struct fw_rsc_vdev_vring consoleTX;
	struct fw_rsc_vdev console2;
	struct fw_rsc_vdev_vring console2RX;
	struct fw_rsc_vdev_vring console2TX;
} PACKED;
struct rtable RESOURCE_TABLE rtable = {
	.resource_table = {
		.ver = 1,
		.num = 3,
	},
	.offset = {
		offsetof(struct rtable, flash),
		offsetof(struct rtable, console),
		offsetof(struct rtable, console2)
	},
	.flash = {
		.type = RSC_CARVEOUT,
		.da = 0x18100000,
		.pa = 0x88100000,
		.len = 0x200000,
		.flags = 0, /* imx has no iommu */
		.name = "flash",
	},
	.console = {
		.type = RSC_VDEV,
		.id = 3, /* Console */
		.notifyid = 0,
		.dfeatures = 0,
		.gfeatures = 0,
		.config_len = 0,
		.status = 0,
		.num_of_vrings = 2
	},
	.consoleRX = {
		.da = 0x88200000,
		.align = 0x1000,
		.num = 256,
		.notifyid = 1,
	},
	.consoleTX = {
		.da = 0x88204000,
		.align = 0x1000,
		.num = 256,
		.notifyid = 2,
	},
	.console2 = {
		.type = RSC_VDEV,
		.id = 3, /* Console */
		.notifyid = 0,
		.dfeatures = 0,
		.gfeatures = 0,
		.config_len = 0,
		.status = 0,
		.num_of_vrings = 2
	},
	.console2RX = {
		.da = 0x88200000,
		.align = 0x1000,
		.num = 256,
		.notifyid = 1,
	},
	.console2TX = {
		.da = 0x88204000,
		.align = 0x1000,
		.num = 256,
		.notifyid = 2,
	},
};
void rprocTest_init() {
	struct rproc *rproc;
	struct mailbox *mbox = mailbox_init(MAILBOX0_ID);
	CONFIG_ASSERT(mbox != NULL);
	rproc = rproc_init(&rprocMailbox_ops, {mbox, mbox}, (struct resource_table *) &rtable, 1, false);
	CONFIG_ASSERT(rproc != NULL);
}
