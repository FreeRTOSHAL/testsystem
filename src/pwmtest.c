#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timer.h>
#include <pwm.h>
#include <remote_control.h>

static int32_t n = 0;
bool crit = false;

void pwmTask(void *data) {
	struct pwm **pwm = data;
	int32_t ret;
	bool up = true;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		crit = true;
		if (up) {
			if (n >= 19500) {
				up = false;
			}
			n += 500;
		} else {
			if (n <= 500) {
				up = true;
			}
			n -= 500;
		}
		ret = pwm_setDutyCycle(pwm[0], n);
		CONFIG_ASSERT(ret == 0);
		ret = pwm_setDutyCycle(pwm[1], n);
		CONFIG_ASSERT(ret == 0);
		ret = pwm_setDutyCycle(pwm[2], n);
		CONFIG_ASSERT(ret == 0);
		crit = false;
		vTaskDelayUntil(&lastWakeUpTime, 20 / portTICK_PERIOD_MS);
	}
}

#ifndef CONFIG_RC
static uint64_t us = 0;
static uint64_t us_old = 0;
int i = 0;
bool start = true;
#define MAX_DIFF 510


void captureIRQ(struct ftm *ftm, void *data, uint32_t channel) {
	uint64_t diff;
	us_old = us;
	us = ftm_getChannelTime(ftm, channel);
	if (!start && !crit && us_old != 0 && us != 0 && us > us_old && n > 1000 && n < 19000) {
		diff = us - us_old;
		CONFIG_ASSERT((diff >= (n - MAX_DIFF) && diff <= (n + MAX_DIFF)) || ((20000 - diff) >= (n - MAX_DIFF) && (20000 - diff) <= (n + MAX_DIFF)));
	} else {
		if (start) {
			i++;
			if (i > 20) {
				start = false;
			}
		}
	}
	//CONFIG_ASSERT((us - 500) >= n && (us + 500) <= n);
}
void overflowIRQ(struct ftm *ftm, void *data) {
	(void) ftm;
	(void) data;
}
#else
void rcTask(void *data) {
	struct rc *rc = data;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	uint32_t time;
	for(;;) {
		time = rc_get(rc, 2);
		printf("rise time at pin: %u\n", (unsigned int) time);
		vTaskDelayUntil(&lastWakeUpTime, 500 / portTICK_PERIOD_MS);
	}
}
#endif

int32_t pwmtest_init() {
	int32_t ret;
	struct timer *ftm_capture = timer_init(1, 32, 20000, 700);
	struct timer *ftm = timer_init(0, 32, 20000, 700);
	struct pwm *pwm[3];
	if (ftm == NULL) {
		return -1;
	}
	ret = timer_periodic(ftm, 20000);
	CONFIG_ASSERT(ret == 0);
	pwm[0] = pwm_init(0, NULL);
	CONFIG_ASSERT(pwm[0] != NULL);
	pwm[1] = pwm_init(2, NULL);
	CONFIG_ASSERT(pwm[1] != NULL);
	pwm[2] = pwm_init(1, NULL);
	CONFIG_ASSERT(pwm[2] != NULL);

	n = 10000;
	
	ret = pwm_setDutyCycle(pwm[0], n);
	CONFIG_ASSERT(ret == 0);
	ret = pwm_setDutyCycle(pwm[1], n);
	CONFIG_ASSERT(ret == 0);
	ret = pwm_setDutyCycle(pwm[2], n);
	CONFIG_ASSERT(ret == 0);
#ifndef CONFIG_RC
	ftm_capture = ftm_init(3, 32, 20000, 700);
	if (ftm_capture == NULL) {
		return -1;
	}
	ret = ftm_periodic(ftm_capture, 30000);
	CONFIG_ASSERT(ret == 0);
	ret = ftm_setOverflowHandler(ftm_capture, &overflowIRQ, NULL);
	CONFIG_ASSERT(ret == 0);
	ret = ftm_setCaptureHandler(ftm_capture, &captureIRQ, NULL);
	CONFIG_ASSERT(ret == 0);
	ret = ftm_setupCapture(ftm_capture, 2);
	CONFIG_ASSERT(ret == 0);
#else
	{
		struct rc *rc = rc_init(ftm_capture);
		struct capture *cap = capture_init(2, NULL);
		CONFIG_ASSERT(cap != NULL);
		ret = rc_setup(rc, cap);
		CONFIG_ASSERT(ret >= 0);
		xTaskCreate(rcTask, "RC Task", 512, rc, 1, NULL);
	}
#endif


	xTaskCreate(pwmTask, "PWM Task", 512, ftm, 1, NULL);
	return 0;
}
