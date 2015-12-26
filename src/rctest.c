#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <remote_control.h>
#include <rctest.h>

void rcTestTask(void *data) {
	struct rc *rc = data;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	uint32_t time;
	for (;;) {
		int i;
		printf("Chanel: ");
		for (i = 0; i < 8; i++) {
			if (i != 1) {
				time = rc_get(rc, i);
				printf("%d: %04" PRIu32 " ", i, time);
			}
		}
		printf("\n");
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
	}
}

void rcInit(struct timer *ftm) {
	int32_t ret;
	struct rc *rc = rc_init(ftm);
	ret = rc_setup(rc, 5);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 4);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 3);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 2);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 7);
	CONFIG_ASSERT(ret == 0);
	ret = rc_setup(rc, 6);
	CONFIG_ASSERT(ret == 0);
	xTaskCreate(rcTestTask, "RC Test Task", 512, rc, 1, NULL);
}
