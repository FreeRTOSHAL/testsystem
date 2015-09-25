#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <flextimer.h>
#include <ppm.h>

#define PPM_MIN_TIME 500
#define PPM_MAX_TIME 1500

struct ppm_slot {
	uint32_t time;
};

enum ppm_state{
	PPM_STOPED,
	PPM_START,
	PPM_DATA,
	PPM_LAST,
	PPM_PAUSE
};

struct ppm {
	uint32_t slots;
	volatile struct ppm_slot *slot;
	struct ftm *timer;
	struct gpio_pin *pin;

	SemaphoreHandle_t sem;
	uint32_t waittime;

	uint32_t pos;
	enum ppm_state state;
	uint32_t sum;
};



static int32_t ppm_startTransfer(struct ppm *ppm) {
	int32_t ret;
	gpio_setPin(ppm->pin);
	ppm->sum -= PPM_MIN_TIME;
	ret = ftm_oneshot(ppm->timer, PPM_MIN_TIME);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static int32_t ppm_data(struct ppm *ppm) {
	int32_t ret;
	int32_t time;
	CONFIG_ASSERT(ppm->pos < ppm->slots);
	time = ppm->slot[ppm->pos].time; 
	ppm->pos++;
	ppm->state = PPM_DATA;
	ppm->sum -= time + PPM_MIN_TIME;
	gpio_clearPin(ppm->pin);
	ret = ftm_oneshot(ppm->timer, time + PPM_MIN_TIME);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

static int32_t ppm_last(struct ppm *ppm) {
	return ppm_startTransfer(ppm);
}
static int32_t ppm_endPause(struct ppm *ppm) {
	int32_t ret;
	ret = ftm_oneshot(ppm->timer, ppm->sum);
	if (ret < 0) {
		return ret;
	}
	return 0;
}
static int32_t ppm_pause(struct ppm *ppm) {
	BaseType_t xHigherPriorityTaskWoken;
	gpio_clearPin(ppm->pin);
	ppm->waittime = (ppm->sum / 1000);
	if (ppm->waittime > 0) {
		ppm->sum -= ppm->waittime * 1000;
		xSemaphoreGiveFromISR(ppm->sem, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else {
		if (ppm->sum > 0) {
			ppm_endPause(ppm);
		} else {
			ppm_start(ppm);
		}
	}
	return 0;
}
void ppm_task(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	struct ppm *ppm = data;
	for(;;) {
		xSemaphoreTake(ppm->sem, portMAX_DELAY);
		if (ppm->state != PPM_STOPED) {
			lastWakeUpTime = xTaskGetTickCount();
			vTaskDelayUntil(&lastWakeUpTime, ppm->waittime);
			if (ppm->sum < 1000) {
				ppm->state = PPM_START;
				ppm_start(ppm);
			} else {
				ppm_endPause(ppm);
			}
		}
	}
}

int32_t ppm_start(struct ppm *ppm) {
	ppm->state = PPM_START;
	ppm->pos = 0;
	ppm->sum = (PPM_MAX_TIME * ppm->slots) + (PPM_MIN_TIME * ppm->slots) + PPM_MIN_TIME;
	return ppm_startTransfer(ppm);
}

static void timerHandler(struct ftm *ftm, void *data) {
	int32_t ret = 0;
	struct ppm *ppm = data;
	switch(ppm->state) {
		case PPM_START:
			ppm->state = PPM_DATA;
			ret = ppm_data(ppm);
			break;
		case PPM_DATA:
			if (ppm->pos < ppm->slots) {
				ppm->state = PPM_START;
				ret = ppm_startTransfer(ppm);
			} else {
				ppm->state = PPM_LAST;
				ret = ppm_last(ppm);
			}
			break;
		case PPM_LAST:
			ppm->state = PPM_PAUSE;
			ret = ppm_pause(ppm);
			break;
		case PPM_PAUSE:
			ppm->state = PPM_START;
			ret = ppm_start(ppm);
			break;
		case PPM_STOPED:	
		default:
			return;
	}
	CONFIG_ASSERT(ret == 0);
}

struct ppm *ppm_init(uint32_t slots, struct gpio_pin *pin) {
	struct ppm *ppm = pvPortMalloc(sizeof(struct ppm));
	if (ppm == NULL) {
		goto ppm_init_error_0;
	}
	ppm->pin = pin;
	ppm->slots = slots;
	ppm->state = PPM_STOPED;
	ppm->pos = 0;
	ppm->slot = (volatile struct ppm_slot *) pvPortMalloc(sizeof(struct ppm_slot) * ppm->slots);
	if (ppm->slot == NULL) {
		goto ppm_init_error_1;
	}
	memset((void *) ppm->slot, 0, ppm->slots);
	ppm->timer = ftm_init(0, 32, timerHandler, ppm);
	if (ppm->timer == NULL) {
		goto ppm_init_error_2;
	}
	gpio_clearPin(pin);
	
	ppm->sem = xSemaphoreCreateBinary();
	if (ppm->sem == NULL) {
		goto ppm_init_error_2;
	}
	xSemaphoreGive(ppm->sem);
	xSemaphoreTake(ppm->sem, portMAX_DELAY);
	xTaskCreate(ppm_task, "PPM Task", 512, ppm, 3, NULL);

	{
		int i = 0;
		for (i = 0; i < ppm->slots; i++) {
			ppm->slot[i].time = 1000;
		}
		ppm->slot[0].time = 0;
		ppm->slot[1].time = 200;
		ppm->slot[2].time = 200;
	}
	return ppm;
ppm_init_error_2:
	vPortFree((void *) ppm->slot);
ppm_init_error_1:
	vPortFree(ppm);
ppm_init_error_0:
	return NULL;
}
