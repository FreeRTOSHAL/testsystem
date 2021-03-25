#include <FreeRTOS.h>
#include <os.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <epwm_test.h>
#include <epwm.h>

bool timer_callback(struct timer *timer, void *data) {

	int32_t ret;
	struct gpio_pin *pin = data;
	ret = gpioPin_togglePin(pin);
	CONFIG_ASSERT(ret >= 0);

	return false;
}


static void epwm_task(void *data) {
	
	int i = 0;
	uint64_t t;
	struct timer **timer = data;
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();
	
	for (;;) { 
		t = timer_getTime(timer);
		PRINTF("%d\n %lu\n", i++, pxPreviousWakeTime);
		PRINTF("get_timer %llu\n", t);
		vTaskDelayUntil(&pxPreviousWakeTime, 1000 / portTICK_PERIOD_MS);
	}
}

OS_DEFINE_TASK(epwmTest, 1000);

void epwm_test() {
	int32_t ret;
	struct gpio_pin *pin = NULL;
	struct gpio *gpio = gpio_init(GPIO_ID);
	static struct timer *timer;
	timer = timer_init(EPWM1_TIMER_ID, 1, 10, 0);
	CONFIG_ASSERT(timer != NULL);
	pin = gpioPin_init(gpio, GPIO_50, GPIO_OUTPUT, GPIO_PULL_UP);
	CONFIG_ASSERT(pin != NULL);
	ret = timer_periodic(timer, 100);
	CONFIG_ASSERT(ret >= 0);
	ret = timer_setOverflowCallback(timer, timer_callback, pin);
	CONFIG_ASSERT(ret >= 0);
	ret = OS_CREATE_TASK(epwm_task, "EPWM Task", 1000, NULL, 2, epwmTest);
	CONFIG_ASSERT(ret == pdPASS);
}
