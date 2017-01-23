#include <softwareCaptureTest.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <stdint.h>
#include <capture.h>
#include <capture_software.h>
#include <rtc.h>
#include <rtc_software.h>
#include <devs.h>
#include <iomux.h>
ADD_RTC_SOFTWARE(0);
ADD_CAPUTRE_SOFTWARE(0);
struct capture *capture;
struct timer *timer;
struct rtc *rtc;
struct gpio_pin *pin;


static bool softwareCaptureTest_callback(struct capture *capture, uint32_t index, uint64_t time, void *data) {
	(void) capture;
	(void) index;
	(void) data;
	printf("%lu: time: %llu", xTaskGetTickCount(), time);
	return false;
}

static void softwareCaptureTest_task(void *data) {
	int32_t ret;
	TickType_t wakeTime = xTaskGetTickCount();
	struct timespec time = {0, wakeTime * 1000};
	ret = rtc_software_connect(rtc, timer);
	CONFIG_ASSERT(ret >= 0);
	ret = rtc_setTime(rtc, &time, portMAX_DELAY);
	CONFIG_ASSERT(ret >= 0);
	ret = capture_software_connect(capture, pin, rtc);
	CONFIG_ASSERT(ret >= 0);
	ret = capture_setCallback(capture, softwareCaptureTest_callback, NULL);
	CONFIG_ASSERT(ret >= 0);
	for (;;) vTaskSuspend(NULL);
}

void softwareCaptureTest_init() {
	struct gpio *gpio = gpio_init(GPIO_ID);
	CONFIG_ASSERT(gpio);
	pin = gpioPin_init(gpio, PAD_MCASP1_AXR13, GPIO_INPUT, GPIO_PULL_DOWN); /* EMPF1 */
	CONFIG_ASSERT(pin);
	timer = timer_init(TIMER2_ID, 1, 1, 0);
	CONFIG_ASSERT(timer);
	rtc = rtc_init(RTC_SOFTWARE_ID(0));
	CONFIG_ASSERT(rtc);
	capture = capture_init(CAPTURE_SOFTWARE_ID(0));
	CONFIG_ASSERT(capture);
	 xTaskCreate(softwareCaptureTest_task, "Capture TestTask", 500, NULL, 2, NULL);
}
