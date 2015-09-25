#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <remote_control.h>
#include <rctest.h>

void rcTask(void *data) {
	struct rc *rc = data;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	uint32_t time;
	for (;;) {
		int i;
		for (i = 0; i < 8; i++) {
			time = rc_get(rc, i);
			printf("Chanel %d: %" PRIu32 "\n", i, time);
		}
		vTaskDelayUntil(&lastWakeUpTime, 500 / portTICK_PERIOD_MS);
	}
}

void rcInit() {
	int32_t ret;
	struct rc *rc = rc_init(0);
	ret = rc_setup(rc, 0);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 1);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 2);
	CONFIG_ASSERT(ret == 0);
	xTaskCreate(rcTask, "RC Test Task", 512, rc, 1, NULL);
}
