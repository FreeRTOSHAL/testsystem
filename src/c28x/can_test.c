#include <stdio.h>
#include <FreeRTOS.h>
#include <os.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <can_test.h>
#include <can.h>

#define PRINTF(...)			printf(__VA_ARGS__)



static bool can_test_msg_received (struct can *can, struct can_msg *msg, void *data) {
	BaseType_t pxHigherPriorityTaskWoken;
	int ret;

	(void) xQueueSendToBackFromISR(data, msg, &pxHigherPriorityTaskWoken);

	ret = can_sendISR(can, msg);
	CONFIG_ASSERT(ret == 0);

	return pxHigherPriorityTaskWoken;
}


static void can_task(void *data) {
	TickType_t pxPreviousWakeTime = xTaskGetTickCount();
	struct can *can0;
	struct can_msg msg;
	int ret;
	int32_t filter_id;
	struct can_filter filter;
	int i;
#ifdef CONFIG_CAN_TEST_ECHO
	OS_DEFINE_QUEUE(can_test_messages, CONFIG_CAN_TEST_ECHO_QUEUE_LENGTH, sizeof(struct can_msg));
#endif

#ifdef CONFIG_CAN_TEST_ECHO
	can_test_messages = OS_CREATE_QUEUE(CONFIG_CAN_TEST_ECHO_QUEUE_LENGTH, sizeof(struct can_msg), can_test_messages);

	can0 = can_init(0, CONFIG_CAN_TEST_BITRATE, NULL, false, NULL, NULL);
	CONFIG_ASSERT(can0);

	ret = can_up(can0);
	CONFIG_ASSERT(ret == 0);

	filter = (struct can_filter) {
		.id = 0x4AB,
		.mask = 0x00F
	};


	filter_id = can_registerFilter(can0, &filter);
	CONFIG_ASSERT(filter_id >= 0);

	can_setCallback(can0, filter_id, can_test_msg_received, can_test_messages);

	for (;;) {
		PRINTF("waiting for incoming messages ...\n");

		ret = xQueueReceive(can_test_messages, &msg, 1000 / portTICK_PERIOD_MS);
		if (ret == pdTRUE) {
			PRINTF("msg received (length=%d):", msg.length);

			for (i=0; i<msg.length; i++) {
				PRINTF(" %02x", msg.data[i]);
			}

			PRINTF("\n");
		}
	}
#endif

#ifdef CONFIG_CAN_TEST_RECV
	can0 = can_init(0, CONFIG_CAN_TEST_BITRATE, NULL, false, NULL, NULL);
	CONFIG_ASSERT(can0);

	ret = can_up(can0);
	CONFIG_ASSERT(ret == 0);

	filter = (struct can_filter) {
		.id = 0x4AB,
		.mask = 0x00F
	};

	filter_id = can_registerFilter(can0, &filter);
	CONFIG_ASSERT(filter_id >= 0);

	for (;;) {
		PRINTF("waiting for incoming messages ...\n");
		ret = can_recv(can0, filter_id, &msg, 1000 / portTICK_PERIOD_MS);
		if (ret == 0) {
			PRINTF("msg received (length=%d):", msg.length);

			for (i=0; i<msg.length; i++) {
				PRINTF(" %02x", msg.data[i]);
			}

			PRINTF("\n");
		} else {
			PRINTF("no message received\n");
		}
	}
#endif

#ifdef CONFIG_CAN_TEST_SEND
	can0 = can_init(0, CONFIG_CAN_TEST_BITRATE, NULL, false, NULL, NULL);
	CONFIG_ASSERT(can0);

	ret = can_up(can0);
	CONFIG_ASSERT(ret == 0);

	msg = (struct can_msg) {
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

