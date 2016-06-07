#include <FreeRTOS.h>
#include <task.h>
#include <gpiotest.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>

void gpiotest_task(void *data) {
	TickType_t lastTime;
	struct gpio_pin *pin = data;
	for(;;) {
		gpioPin_togglePin(pin);
		vTaskDelayUntil(&lastTime, 10 / portTICK_PERIOD_MS);
	}
}

void gpiotest_init() {
	struct gpio *gpio = gpio_init(GPIO_ID);
	struct gpio_pin *pin;
	CONFIG_ASSERT(gpio != NULL);
	pin = gpioPin_init(gpio, PIN_0_7, GPIO_OUTPUT, GPIO_PULL_UP);
	CONFIG_ASSERT(pin != NULL);
	xTaskCreate(gpiotest_task, "GPIO Test Task", 100, pin, 1, NULL);
}
