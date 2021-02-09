#include <FreeRTOS.h>
#include <os.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <epwm_test.h>
#include <epwm.h>

static void epwm_task(void *data) {
	int i = 0;
	uint64_t t;
	PRINTF("timer start\n");
	struct timer *timer;

	timer = timer_init(EPWM1_TIMER_ID, 3, 10, 0);
	CONFIG_ASSERT(timer != NULL);
	
	timer_start(timer);
	timer_periodic(timer, 10000);
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();
	for (;;) {
		t = timer_getTime(timer);
		timer_getTime(timer);
		PRINTF("%d\n %lu\n", i++, pxPreviousWakeTime);
		PRINTF("get_timer %llu\n", t);
		vTaskDelayUntil(&pxPreviousWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

OS_DEFINE_TASK(epwmTest, 1000);

void epwm_test() {
	int32_t ret;
	ret = OS_CREATE_TASK(epwm_task, "EPWM Task", 1000, NULL, 2, epwmTest);
	CONFIG_ASSERT(ret == pdPASS);
}
