#include <FreeRTOS.h>
#include <os.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <can_test.h>
#include <can.h>

static void can_task(void *data) {
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();

#ifdef CONFIG_CAN_TEST_SEND
	struct can* can0 = can_init(0, CONFIG_CAN_TEST_BITRATE, NULL, false, NULL, NULL);
	CONFIG_ASSERT(can0);

	struct can_msg msg = {
		.id = 0x4AB,
		.length = 8,
		.data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}
	};

	for (;;) {
		can_send(can0, &msg, 1000 / portTICK_PERIOD_MS);
		vTaskDelayUntil(&pxPreviousWakeTime, 1000 / portTICK_PERIOD_MS);
	}
#endif

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

OS_DEFINE_TASK(canTest, 2048);

void can_test() {
	int32_t ret;
	ret = OS_CREATE_TASK(can_task, "CAN Task", 2048, NULL, 2, canTest);
	CONFIG_ASSERT(ret == pdPASS);
}


// vim: noexpandtab ts=4 sts=4 sw=4

