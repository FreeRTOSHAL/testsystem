#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <remote_control.h>
#include <flextimer.h>
struct rc {
	uint32_t channelValue[RC_MAX_CHANNELS];
	uint32_t channelValueRAW[RC_MAX_CHANNELS];
	struct ftm *ftm;
};


static void rc_IRQHandler(struct ftm *ftm, void *data, uint32_t channel) {
	struct rc *rc = data;
	uint32_t tmp_1 = rc->channelValueRAW[channel];
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
		rc->channelValue[channel] = tmp;
	}

	/* Save old Value for next Calculation */
	rc->channelValueRAW[channel] = tmp_0;
}

struct rc *rc_init(uint32_t ftmid) {
	int32_t ret;
	struct rc *rc = pvPortMalloc(sizeof(struct rc));
	if (rc == NULL) {
		goto rc_init_error_0;
	}
	memset(rc, 0, sizeof(struct rc));
	rc->ftm = ftm_init(ftmid, 32, NULL, rc, 20000, 700);
	if (rc->ftm == NULL) {
		goto rc_init_error_1;
	}
	ret = ftm_periodic_capture(rc->ftm, rc_IRQHandler);
	if (ret < 0) {
		goto rc_init_error_2;
	}
	return rc;
rc_init_error_2:
	//ftm_deinit(ftm); 
#warning TODO FTM deinit!
rc_init_error_1:
	vPortFree(rc);
rc_init_error_0:
	return NULL;
}

int32_t rc_setup(struct rc *rc, uint32_t channel) {
	rc->channelValue[channel] = 0;
	rc->channelValueRAW[channel] = 0;
	return ftm_setupCapture(rc->ftm, channel);
}
uint32_t rc_get(struct rc *rc, uint32_t channel) {
	return rc->channelValue[channel];
}
