#include <FreeRTOS.h>
#include <os.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <can_test.h>

static void can_task(void *data) {
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();
#ifdef CONFIG_CAN_TEST_TOGGLE_PIN
	struct gpio *gpio = gpio_init(GPIO_ID);
	struct gpio_pin *cantxa = gpioPin_init(gpio, GPIO_31, GPIO_OUTPUT, GPIO_PULL_UP);
	struct gpio_pin *canrxa = gpioPin_init(gpio, GPIO_30, GPIO_INPUT, GPIO_OPEN);
	bool value = true;
	CONFIG_ASSERT(cantxa);
	CONFIG_ASSERT(canrxa);
	gpioPin_setPin(cantxa);
#endif
	for (;;) {
#ifdef CONFIG_CAN_TEST_TOGGLE_PIN
		printf("Toggle CAN Pin\n");		
		gpioPin_togglePin(cantxa);
		value = !value;
		CONFIG_ASSERT(gpioPin_getValue(canrxa) == value);
#endif
		vTaskDelayUntil(&pxPreviousWakeTime, 10 / portTICK_PERIOD_MS);
	}
}

OS_DEFINE_TASK(canTest, 1000);

void can_test() {
	int32_t ret;
	ret = OS_CREATE_TASK(can_task, "CAN Task", 1000, NULL, 2, canTest);
	CONFIG_ASSERT(ret == pdPASS);
}
