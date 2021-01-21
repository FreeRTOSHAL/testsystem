#include <can.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>
#include <stdio.h>
#include <iomux.h>
#include <gpio.h>

OS_DEFINE_TASK(taskCAN, 512);

void dcan1_gpio(){
	int i;
	struct gpio *gpio = gpio_init(GPIO_ID);
	struct gpio_pin *cantx = gpioPin_init(gpio,PAD_DCAN1_TX, GPIO_OUTPUT, GPIO_PULL_DOWN);
	CONFIG_ASSERT(cantx);
	//gpioPin_clearPin(cantx);
	gpioPin_setPin(cantx);

}

void dcan_gpio_task(void *data){
	int i;
	struct gpio *gpio = gpio_init(GPIO_ID);
	struct gpio_pin *cantx = gpioPin_init(gpio,PAD_DCAN1_TX, GPIO_OUTPUT, GPIO_PULL_UP);
	TickType_t lastWakeUpTime;
	/*
	struct gpio_pin *canrx = gpioPin_init(gpio, PAD_DCAN1_RX, GPIO_INPUT, GPIO_PULL_DOWN);
	TickType_t waittime = 100;
	int rx;
	CONFIG_ASSERT(cantx);
	rx = gpioPin_getValue(canrx);
	printf("CAN RX PIN value before clear:	 %i\n", rx);
	vTaskDelay(waittime / portTICK_PERIOD_MS);
	gpioPin_clearPin(cantx);
	rx = gpioPin_getValue(canrx);
	printf("CAN RX PIN value after clear:	%i\n", rx);
	vTaskDelay(waittime / portTICK_PERIOD_MS);
	gpioPin_setPin(cantx);
	rx = gpioPin_getValue(canrx);
	printf("CAN RX PIN value after setPin:	 %i\n", rx);
	vTaskDelay(waittime / portTICK_PERIOD_MS);
	printf("start printing CAN RX value\n");
	for(;;){
		rx = gpioPin_getValue(canrx);
		printf("CAN RX PIN value:	%i\n", rx);
		vTaskDelay(waittime / portTICK_PERIOD_MS);
	}
	*/
	lastWakeUpTime = xTaskGetTickCount();
	for(;;){

		gpioPin_clearPin(cantx);
		vTaskDelayUntil(&lastWakeUpTime, 10/ portTICK_PERIOD_MS);
		gpioPin_setPin(cantx);
		vTaskDelayUntil(&lastWakeUpTime, 10/ portTICK_PERIOD_MS);
	}


	for(;;);
}



void can_task(void *data) {
	struct can *can1 = data;
	struct can_msg msg_send = {0, 2, 0, 8, {4,20, 69, 66, 21, 100, 13, 37}};
	struct can_msg msg_recv;
	struct can_filter filter = {3, 0};
	int filterID, i;
	int32_t ret;
	memset(&msg_recv, 0xFF, sizeof(struct can_msg));
	filterID = can_registerFilter(can1, &filter);
	CONFIG_ASSERT(filterID >= 0);
	ret = can_send(can1, &msg_send, (1000 / portTICK_PERIOD_MS));
	CONFIG_ASSERT(ret == 0);

	/*
	   ret = can_recv(can1,filterID, &msg_recv, (1000 / portTICK_PERIOD_MS));
	   CONFIG_ASSERT(ret == 0);
	   printf("can_test: received msg\nts: %i\nid: %i\nreq: %i\nlength: %i\n", msg_recv.ts, msg_recv.id, msg_recv.req, msg_recv.length);
	   for(i = 0; i < msg_recv.length; ++i){
	   printf("data[%i]: %i\n", i, msg_recv.data[i]);
	   }
	   */



	/*
	for(;;){
		ret = can_send(can1, &msg_send, (1000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
		ret = can_recv(can1,filterID, &msg_recv, (10000 / portTICK_PERIOD_MS));
		CONFIG_ASSERT(ret == 0);
		CONFIG_ASSERT(msg_recv.id == msg_send.id);
		CONFIG_ASSERT(msg_recv.length == msg_send.length);
		//CONFIG_ASSERT(msg_recv.req == msg_send.req);
		for (i = 0; i < 8; i++) {
			CONFIG_ASSERT(msg_recv.data[i] == msg_send.data[i]);
		}


	}
	*/

	//can_deregisterFilter(can1, filterID);
	//can_deinit(can1);
	

	for(;;){
		ret = can_recv(can1, filterID, &msg_recv, (10000 / portTICK_PERIOD_MS));
		if(ret == 0){
			printf("can_test: received msg\nts: %#08lx\nid: %#08lx\nreq: %#08lx\nlength: %#08lx\n", 
					msg_recv.ts, msg_recv.id, msg_recv.req, msg_recv.length);
			for(i = 0; i < msg_recv.length; ++i){
				printf("data[%i]: %#08lx\n", i, msg_recv.data[i]);
			}
		}
		else {
			printf("can_test: received nothing\n");
		}
	}
	
	printf("can_test finished\n\n");
	for(;;);
}

void can_test() {
	struct can *can = can_init(DCAN1_ID, 500000, NULL, false, false, NULL);
	OS_CREATE_TASK(can_task, "CAN Test", 512, can, 2, taskCAN);
	//OS_CREATE_TASK(dcan_gpio_task, "CAN Test", 512, NULL, 2, taskCAN);
	//dcan1_gpio();

}
// vim: noexpandtab ts=4 sts=4 sw=4
