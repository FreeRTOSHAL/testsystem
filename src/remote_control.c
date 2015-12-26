#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <remote_control.h>
#include <flextimer.h>
struct rc_channel {
	uint32_t raw;
	uint32_t value;
	bool retry;
	bool corret;
};
struct rc {
	struct rc_channel channel[RC_MAX_CHANNELS];
	struct timer *ftm;
};


static bool rc_IRQHandler(struct timer *ftm, void *data, uint32_t channel) {
	struct rc *rc = data;
	uint32_t tmp_1 = rc->channel[channel].raw;
	uint32_t tmp_0 = ftm_getChannelTime(ftm, channel);
	uint32_t tmp;

	if (tmp_0 > tmp_1) {
		tmp = tmp_0 - tmp_1;
	} else {
		/* 
		 * Overflow
		 */
		tmp = (UINT16_MAX - tmp_1) + tmp_0;
	}
	/* 
	 * Detect is Short or Long Impulse 
	 * PPM Motor / Servo Signal is approx 20ms long
	 * The Impulses is at the beginning of the signal
	 * The signal is min 0.5 ms and max 2 ms
	 * 
	 * The Interrupt is generated at begging of 
	 * rising edge and at falling edge 
	 * 
	 *1 |  ___         ___
	 *  |  |  |        |  |
	 *0 |__|  |___...__|  |
	 *  +--|--|--|--|--|--|
	 *  0  1  2  3 ... 20ms
	 *      ^ Impulse    ^ Impulse   
	 */
	if (tmp > RC_MIN && tmp < RC_MAX) {
		rc->channel[channel].value = tmp;
		rc->channel[channel].corret = true;
		rc->channel[channel].retry = true;
	}

	/* Save old Value for next Calculation */
	rc->channel[channel].raw = tmp_0;
	return 0;
}

static bool rc_OverfowHandler(struct timer *ftm, void *data) {
	struct rc *rc = data;
	int i;
	for (i = 0; i < RC_MAX_CHANNELS; i++) {
		/* 
		 * Wait unil next Overfow if !retry = value is not corect
		 */
		if (rc->channel[i].retry) {
			rc->channel[i].retry = false;
		} else {
			rc->channel[i].corret = false;
		}
	}
	return 0;
}


struct rc *rc_init(struct timer *ftm) {
	int32_t ret;
	struct rc *rc = pvPortMalloc(sizeof(struct rc));
	if (rc == NULL) {
		goto rc_init_error_0;
	}
	memset(rc, 0, sizeof(struct rc));
	rc->ftm = ftm;
	ret = timer_setOverflowCallback(rc->ftm, rc_OverfowHandler, rc);
	if (ret < 0) {
		goto rc_init_error_1;
	}
	ret = ftm_setCaptureHandler(rc->ftm, rc_IRQHandler, rc);
	if (ret < 0) {
		goto rc_init_error_1;
	}
	return rc;
rc_init_error_1:
	vPortFree(rc);
rc_init_error_0:
	return NULL;
}

int32_t rc_setup(struct rc *rc, uint32_t channel) {
	rc->channel[channel].raw = 0;
	rc->channel[channel].value = 0;
	rc->channel[channel].retry = 0;
	rc->channel[channel].corret = 0;
	return ftm_setupCapture(rc->ftm, channel);
}
uint32_t rc_get(struct rc *rc, uint32_t channel) {
	if (rc->channel[channel].corret) {
		return rc->channel[channel].value;
	} else {
		return 0;
	}
}
