#include <can.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>

OS_DEFINE_TASK(taskCAN, 512);

#ifdef CONFIG_FLEXCAN_ECHO_TEST
void can_task(void *data) {
	struct can *can = data;
	int32_t ret;
	int32_t filterID;
	struct can_msg msg = {
		.id = 0x123,
		.length = 8,
		.data = {
			0x01,
			0x12,
			0x34,
			0x56,
			0x78,
			0x9A,
			0xAB,
			0xCD,
		},
	};
	struct can_msg msg_recv;
	struct can_filter filter = {
		.id = 0x123,
		.mask = 0xFF0,
	};
	int i;
	memset(&msg_recv, 0xFF, sizeof(struct can_msg));
	ret = can_up(can);
	CONFIG_ASSERT(ret == 0);
	filterID = can_registerFilter(can, &filter);
	CONFIG_ASSERT(filterID != -1);
	for (;;) {
		ret = can_send(can, &msg, (1000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
		ret = can_recv(can, filterID, &msg_recv, (1000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
		CONFIG_ASSERT(msg_recv.id == msg.id);
		CONFIG_ASSERT(msg_recv.length == msg.length);
		for (i = 0; i < 8; i++) {
			CONFIG_ASSERT(msg_recv.data[i] == msg.data[i]);
		}
	}
}
#else
void can_task(void *data) {
	struct can *can = data;
	int32_t ret;
	int32_t filterID;
	struct can_msg msg;
	struct can_filter filter = {
		.id = 0x123,
		.mask = 0xFF0,
	};
	memset(&msg, 0xFF, sizeof(struct can_msg));
	ret = can_up(can);
	CONFIG_ASSERT(ret == 0);
	filterID = can_registerFilter(can, &filter);
	CONFIG_ASSERT(filterID != -1);
	for (;;) {
		ret = can_recv(can, filterID, &msg, portMAX_DELAY);
		CONFIG_ASSERT(ret == 0);
		//printf("0x%x(%d): %0x%0x%0x%0x%0x%0x%0x%0x\n", msg.id, msg.length, msg.data[0], msg.data[1], msg.data[2], msg.data[3],  msg.data[4], msg.data[5], msg.data[6], msg.data[7]);
		msg.id -= 0x100;
		ret = can_send(can, &msg, (1000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
	}
}
#endif

bool error_callback(struct can *can, can_error_t error, can_errorData_t data, void *userData) {
	printf("CAN Error / Waring: error: %lu data: %llu\n", error, data);
	return false;
}

void can_test() {
	struct can *can = can_init(FLEXCAN0_ID, 500000, NULL, false, error_callback, NULL);
	CONFIG_ASSERT(can);

	OS_CREATE_TASK(can_task, "CAN Test", 512, can, 2, taskCAN);
}
