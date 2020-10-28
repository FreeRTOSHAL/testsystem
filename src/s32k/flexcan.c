#include <can.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>

OS_DEFINE_TASK(taskCAN, 512);

void can_task(void *data) {
	struct can *can = data;
	int32_t ret;
	int32_t filterID;
	struct can_msg msg = {
		.id = 0x123,
		.req = false,
		.length = 8,
		.data = {
			0x12,
			0x12,
			0x12,
			0x12,
			0x12,
			0x12,
			0x12,
			0x12,
		},
	};
	struct can_msg msg_recv;
	struct can_filter filter = {
		.id = 0x123,
		.mask = 0xFF0,
	};
	int i;
	memset(&msg_recv, 0xFF, sizeof(struct can_msg));
	filterID = can_registerFilter(can, &filter);
	CONFIG_ASSERT(filterID != -1);
	ret = can_up(can);
	CONFIG_ASSERT(ret == 0);
	for (;;) {
		ret = can_send(can, &msg, (1000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
		ret = can_recv(can, filterID, &msg_recv, (1000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
		CONFIG_ASSERT(msg_recv.id == msg.id);
		CONFIG_ASSERT(msg_recv.length == msg.length);
		CONFIG_ASSERT(msg_recv.req == msg.req);
		for (i = 0; i < 8; i++) {
			CONFIG_ASSERT(msg_recv.data[i] == msg.data[i]);
		}
	}
}

void can_test() {
	struct can *can = can_init(FLEXCAN0_ID, 500000, NULL, false);
	CONFIG_ASSERT(can);

	OS_CREATE_TASK(can_task, "CAN Test", 512, can, 2, taskCAN);
}
