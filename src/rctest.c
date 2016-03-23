/*
 * Copyright (c) 2016 Andreas Werner <kernel@andy89.org>
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */
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
			time = rc_get(rc, i);
			printf("%d: %04" PRIu32 " ", i, time);
		}
		printf("\n");
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
	}
}

void rcInit(struct timer *ftm) {
	int32_t ret;
	struct rc *rc = rc_init(ftm);
	struct capture *cap[6];
	cap[0] = capture_init(5);
	CONFIG_ASSERT(cap[0] != NULL);
	cap[1] = capture_init(4);
	CONFIG_ASSERT(cap[1] != NULL);
	cap[2] = capture_init(3);
	CONFIG_ASSERT(cap[2] != NULL);
	cap[3] = capture_init(2);
	CONFIG_ASSERT(cap[3] != NULL);
	cap[4] = capture_init(7);
	CONFIG_ASSERT(cap[4] != NULL);
	cap[5] = capture_init(6);
	CONFIG_ASSERT(cap[5] != NULL);
	ret = rc_setup(rc, cap[0]);
	CONFIG_ASSERT(ret >= 0);
	ret = rc_setup(rc, cap[1]);
	CONFIG_ASSERT(ret >= 0);
	ret = rc_setup(rc, cap[2]);
	CONFIG_ASSERT(ret >= 0);
	ret = rc_setup(rc, cap[3]);
	CONFIG_ASSERT(ret >= 0);
	ret = rc_setup(rc, cap[4]);
	CONFIG_ASSERT(ret >= 0);
	ret = rc_setup(rc, cap[5]);
	CONFIG_ASSERT(ret >= 0);
	xTaskCreate(rcTestTask, "RC Test Task", 512, rc, 1, NULL);
}
