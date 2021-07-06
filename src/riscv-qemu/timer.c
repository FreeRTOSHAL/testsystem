/* SPDX-License-Identifier: MIT */
/*
 * Author: Andreas Werner <kernel@andy89.org>
 * Date: 2021
 */
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <timer.h>
#include <devs.h>

struct timer *timer;

static bool irqhandle(struct timer *timer, void *data) {
	(void) data;
	printf("callback\n");
	return false;
}

static void timerTask(void *data) {
	int32_t ret;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	uint64_t time;
	uint32_t timeh;
	uint32_t timel;
	timer = timer_init(TIMER0_ID, 1, 1, 0);
	CONFIG_ASSERT(timer != NULL);
	ret = timer_setOverflowCallback(timer, &irqhandle, NULL);
	CONFIG_ASSERT(ret == 0);
	CONFIG_ASSERT(timer_periodic(timer, 1000000ULL) == 0);
	for (;;) {
		time = timer_getTime(timer);
		timeh = time >> 32;
		timel = time & 0xFFFFFFFFULL;
		printf("time: %lu %lu\n", timeh , timel);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}


OS_DEFINE_TASK(taskTimer, 512);
int32_t timertest_init() {
	BaseType_t bret;
	printf("Timer Test Start\n");
	bret = OS_CREATE_TASK(timerTask, "timer", 512, NULL, 2, taskTimer);
	return 0;
}

