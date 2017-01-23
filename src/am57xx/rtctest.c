#include <FreeRTOS.h>
#include <task.h>
#include <rtc.h>
#include <rtc_software.h>
#include <devs.h>
ADD_RTC_SOFTWARE(0);
struct timer *timer;
struct rtc *rtc;
static void rtctest_task(void *data) {
	int32_t ret;
	TickType_t wakeTime = xTaskGetTickCount();
	struct timespec time = {0, wakeTime * 1000};
	(void) data;
	ret = rtc_software_connect(rtc, timer);
	CONFIG_ASSERT(ret >= 0);
	ret = rtc_setTime(rtc, &time, portMAX_DELAY);
	CONFIG_ASSERT(ret >= 0);
	for (;;) {
		ret = rtc_getTime(rtc, &time, portMAX_DELAY);
		CONFIG_ASSERT(ret >= 0);
		printf("%lu: sec: %ld nsec: %lu\n", wakeTime, time.tv_sec, time.tv_nsec);
		vTaskDelayUntil(&wakeTime, 500 / portTICK_PERIOD_MS);
	}
}

void rtctest_init() {
	timer = timer_init(TIMER2_ID, 1, 1, 0);
	CONFIG_ASSERT(timer);
	rtc = rtc_init(RTC_SOFTWARE_ID(0));
	CONFIG_ASSERT(rtc);
	xTaskCreate(rtctest_task, "RTC TestTask", 500, NULL, 2, NULL);
}
