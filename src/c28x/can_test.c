#include <stdio.h>
#include <FreeRTOS.h>
#include <os.h>
#include <gpio.h>
#include <devs.h>
#include <iomux.h>
#include <can_test.h>
#include <can.h>

#define PRINTF(...)			printf(__VA_ARGS__)


const char *CAN_ERR_NAMES[] = {
	"CAN_ERR_CTRL",
	"CAN_ERR_PROT",
	"CAN_ERR_TRX"
	"CAN_ERR_ACK",
	"CAN_ERR_BUSOFF",
	"CAN_ERR_BUSERROR",
	"CAN_ERR_RESTARTED"
};

const char *CAN_ERR_DATA_NAMES[] = {
	// controller
	// offset: 0
	"CAN_ERR_CRTL_UNSPEC",
	"CAN_ERR_CRTL_RX_OVERFLOW",
	"CAN_ERR_CRTL_TX_OVERFLOW",
	"CAN_ERR_CRTL_RX_WARNING",
	"CAN_ERR_CRTL_TX_WARNING",
	"CAN_ERR_CRTL_RX_PASSIVE",
	"CAN_ERR_CRTL_TX_PASSIVE",
	"CAN_ERR_CRTL_ACTIVE",

	// protocol
	// offset: 8
	"CAN_ERR_PROT_UNSPEC",
	"CAN_ERR_PROT_BIT",
	"CAN_ERR_PROT_FORM",
	"CAN_ERR_PROT_STUFF",
	"CAN_ERR_PROT_BIT0",
	"CAN_ERR_PROT_BIT1",
	"CAN_ERR_PROT_OVERLOAD",
	"CAN_ERR_PROT_ACTIVE",
	"CAN_ERR_PROT_TX",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",

	// protocol (location)
	// offset: 24
	"CAN_ERR_PROT_LOC_UNSPEC",
	"CAN_ERR_PROT_LOC_SOF",
	"CAN_ERR_PROT_LOC_ID28_21",
	"CAN_ERR_PROT_LOC_ID20_18",
	"CAN_ERR_PROT_LOC_SRTR",
	"CAN_ERR_PROT_LOC_IDE",
	"CAN_ERR_PROT_LOC_ID17_13",
	"CAN_ERR_PROT_LOC_ID12_05",
	"CAN_ERR_PROT_LOC_ID04_00",
	"CAN_ERR_PROT_LOC_RTR",
	"CAN_ERR_PROT_LOC_RES1",
	"CAN_ERR_PROT_LOC_RES0",
	"CAN_ERR_PROT_LOC_DLC",
	"CAN_ERR_PROT_LOC_DATA",
	"CAN_ERR_PROT_LOC_CRC_SEQ",
	"CAN_ERR_PROT_LOC_CRC_DEL",
	"CAN_ERR_PROT_LOC_ACK",
	"CAN_ERR_PROT_LOC_ACK_DEL",
	"CAN_ERR_PROT_LOC_EOF",
	"CAN_ERR_PROT_LOC_INTERM",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",
	"CAN_ERR_UNKNOWN",

	// transceiver
	// offset: 48
	"CAN_ERR_TRX_UNSPEC",
	"CAN_ERR_TRX_CANH_NO_WIRE",
	"CAN_ERR_TRX_CANH_SHORT_TO_BAT",
	"CAN_ERR_TRX_CANH_SHORT_TO_VCC",
	"CAN_ERR_TRX_CANH_SHORT_TO_GND",
	"CAN_ERR_TRX_CANL_NO_WIRE",
	"CAN_ERR_TRX_CANL_SHORT_TO_BAT",
	"CAN_ERR_TRX_CANL_SHORT_TO_VCC",
	"CAN_ERR_TRX_CANL_SHORT_TO_GND",
	"CAN_ERR_TRX_CANL_SHORT_TO_CANH",
};


static void print_can_error (can_error_t err, can_errorData_t err_data) {
	int i;

	for (i=0; err; i++) {
		if (err & 1) {
			if (i < ARRAY_SIZE(CAN_ERR_NAMES)) {
				PRINTF("CAN error: %s\n", CAN_ERR_NAMES[i]);
			} else {
				PRINTF("CAN error: unknown, bit=%d\n", i);
			}
		}

		err >>= 1;
	}

	for (i=0; err_data; i++) {
		if (err_data & 1) {
			if (i < ARRAY_SIZE(CAN_ERR_DATA_NAMES)) {
				PRINTF("CAN error (data): %s\n", CAN_ERR_DATA_NAMES[i]);
			} else {
				PRINTF("CAN error (data): unknown, bit=%d\n", i);
			}
		}

		err_data >>= 1;
	}
}




can_error_t can_err = 0;
can_errorData_t can_err_data = 0;


static bool can_test_msg_received (struct can *can, struct can_msg *msg, void *data) {
	BaseType_t pxHigherPriorityTaskWoken;
	int ret;

	(void) xQueueSendToBackFromISR(data, msg, &pxHigherPriorityTaskWoken);

	ret = can_sendISR(can, msg);
	CONFIG_ASSERT(ret == 0);

	return pxHigherPriorityTaskWoken;
}

static bool can_test_error (struct can *can, can_error_t err, can_errorData_t data, void *user_data) {
	can_err |= err;
	can_err_data |= data;

	return false;
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

	can0 = can_init(ECAN0_ID, CONFIG_CAN_TEST_BITRATE, NULL, false, can_test_error, NULL);
	CONFIG_ASSERT(can0);

	ret = can_up(can0);
	CONFIG_ASSERT(ret == 0);

	filter = (struct can_filter) {
		.id = 0x4ABUL,
		.mask = 0xFFCUL
	};


	filter_id = can_registerFilter(can0, &filter);
	CONFIG_ASSERT(filter_id >= 0);

	can_setCallback(can0, filter_id, can_test_msg_received, can_test_messages);

	filter = (struct can_filter) {
		.id = CAN_EFF_FLAG | 0x002C0000UL,
		.mask = 0x002FFFFEUL
	};


	filter_id = can_registerFilter(can0, &filter);
	CONFIG_ASSERT(filter_id >= 0);

	can_setCallback(can0, filter_id, can_test_msg_received, can_test_messages);

	for (;;) {
		PRINTF("waiting for incoming messages ...\n");

		ret = xQueueReceive(can_test_messages, &msg, 1000 / portTICK_PERIOD_MS);
		if (ret == pdTRUE) {
			if (!(msg.id & CAN_EFF_FLAG)) {
				PRINTF("msg received from %03lx @ t=%lu (length=%d):", msg.id & CAN_SFF_MASK, msg.ts, msg.length);
			} else {
				PRINTF("msg received from %08lx @ t=%lu (length=%d):", msg.id & CAN_EFF_MASK, msg.ts, msg.length);
			}

			for (i=0; i<msg.length; i++) {
				PRINTF(" %02x", msg.data[i]);
			}

			PRINTF("\n");
		}

		if (can_err) {
			asm(" DINT");
			uint64_t tmp_can_err = can_err;
			uint64_t tmp_can_err_data = can_err_data;
			can_err = 0;
			can_err_data = 0;
			asm(" EINT");

			print_can_error(tmp_can_err, tmp_can_err_data);
		}
	}
#endif

#ifdef CONFIG_CAN_TEST_RECV
	can0 = can_init(ECAN0_ID, CONFIG_CAN_TEST_BITRATE, NULL, false, can_test_error, NULL);
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

		if (can_err) {
			asm(" DINT");
			uint64_t tmp_can_err = can_err;
			uint64_t tmp_can_err_data = can_err_data;
			can_err = 0;
			can_err_data = 0;
			asm(" EINT");

			print_can_error(tmp_can_err, tmp_can_err_data);
		}
	}
#endif

#ifdef CONFIG_CAN_TEST_SEND
	can0 = can_init(ECAN0_ID, CONFIG_CAN_TEST_BITRATE, NULL, false, can_test_error, NULL);
	CONFIG_ASSERT(can0);

	ret = can_up(can0);
	CONFIG_ASSERT(ret == 0);

	msg = (struct can_msg) {
		.id = 0x4AB,
		.length = 8,
		.data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}
	};

	for (;;) {
		PRINTF("transmit message ...\n");
		ret = can_send(can0, &msg, 1000 / portTICK_PERIOD_MS);
		if (ret < 0) {
			PRINTF("error: send failed!\n");
		}

		if (can_err) {
			asm(" DINT");
			uint64_t tmp_can_err = can_err;
			uint64_t tmp_can_err_data = can_err_data;
			can_err = 0;
			can_err_data = 0;
			asm(" EINT");

			print_can_error(tmp_can_err, tmp_can_err_data);
		}

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

