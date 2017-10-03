/*
 * Copyright (c) 2016 Andreas Werner <kernel@andy89.org>
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <buffertest.h>
#include <buffer.h>
#define BUFFER_PRV
#include <buffer_prv.h>
#include <os.h>

struct buffer *buffer1 = NULL;
struct buffer *buffer2 = NULL;
struct buffer *buffer3 = NULL;
struct buffer *buffer4 = NULL;
char buffer[(128 + sizeof(struct buffer_base))];
void buffer1Test(void *data);
void buffer2Test(void *data);
void buffer3Test(void *data);
void buffer4Test(void *data);
#ifdef CONFIG_BUFFERTEST_IRQ
static void bufferIRQTest(void *data);
#endif
#ifdef CONFIG_TTY_BUFFERTEST
static void bufferTTYTest(void *data);
#endif
struct TestData {
	char test[31];
	bool value;
};
char buffer_2[(32 * sizeof(struct TestData) + sizeof(struct buffer_base))];
SemaphoreHandle_t firstTest_0 = NULL;
SemaphoreHandle_t firstTest_1 = NULL;

#define BUFFER_UART_RX ((struct buffer_base *) 0x3f07fbff)
#define BUFFER_UART_TX ((struct buffer_base *) 0x3f07fc97)

struct buffer *bufferRX;
struct buffer *bufferTX;
#ifdef CONFIG_BUFFERTEST_SELF
OS_DEFINE_TASK(buffer1TestTask, 512);
OS_DEFINE_TASK(buffer2TestTask, 512);
#endif
#ifdef CONFIG_BUFFERTEST_IRQ
OS_DEFINE_TASK(bufferIRQTask, 512);
#endif
#ifdef CONFIG_TTY_BUFFERTEST
OS_DEFINE_TASK(bufferTTYTask, 512);
#endif
void bufferInit() {
#ifdef CONFIG_BUFFERTEST_SELF
	buffer1 = buffer_init((struct buffer_base *) buffer, 128, sizeof(char), false, 1);
	CONFIG_ASSERT(buffer1 != NULL);
	buffer2 = buffer_init((struct buffer_base *) buffer, 128, sizeof(char), true, 1);
	CONFIG_ASSERT(buffer2 != NULL);
	OS_CREATE_TASK(buffer1Test, "Buffer Test", 512, NULL, 2, buffer1TestTask);
	OS_CREATE_TASK(buffer2Test, "Buffer 2 Test", 512, NULL, 3, buffer2TestTask);
#endif
	bufferRX = buffer_init(BUFFER_UART_RX, 128, sizeof(char), true, 1);
	CONFIG_ASSERT(bufferRX != NULL);
	bufferTX = buffer_init(BUFFER_UART_TX, 128, sizeof(char), false, 1);
	CONFIG_ASSERT(bufferTX != NULL);
#ifdef CONFIG_BUFFERTEST_IRQ
	OS_CREATE_TASK( bufferIRQTest, "Buffer IRQ Test", 512, NULL, 2, bufferIRQTask);
#endif
#ifdef CONFIG_TTY_BUFFERTEST
	OS_CREATE_TASK(bufferTTYTest, "Buffer TTY Test", 512, NULL, 2, bufferTTYTask);
#endif
}

#ifdef CONFIG_BUFFERTEST_SELF
OS_DEFINE_TASK(buffer3TestTask, 512);
OS_DEFINE_TASK(buffer4TestTask, 512);
void bufferInit2() {
	buffer3 = buffer_init((struct buffer_base *) buffer_2, 32, sizeof(struct TestData), false, 1);
	buffer4 = buffer_init((struct buffer_base *) buffer_2, 32, sizeof(struct TestData), true, 1);
	OS_CREATE_TASK(buffer3Test, "Buffer 3 Test", 512, NULL, 2, buffer3TestTask);
	OS_CREATE_TASK(buffer4Test, "Buffer 4 Test", 512, NULL, 3, buffer4TestTask);
}

void buffer1Test(void *data) {
	int i;
	int32_t ret;
	char test[] = "Dies ist ein Test";
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (i = 0; i < 10; i++) {
		ret = buffer_write(buffer1, (uint8_t *) test, strlen(test) + 1);
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		vTaskDelayUntil(&lastWakeUpTime, 1 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void buffer2Test(void *data) {
	int i;
	int32_t ret;
	char test[128];
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (i = 0; i < 10; i++) {
		ret = buffer_read(buffer2, (uint8_t *) test, 128, portMAX_DELAY);
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		printf("recv: %s len: %ld\n", test,  ret);
		if (ret != 18) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		vTaskDelayUntil(&lastWakeUpTime, 1 / portTICK_PERIOD_MS);
	}
	bufferInit2();
	vTaskDelete(NULL);
}

void buffer3Test(void *data) {
	int i;
	int32_t ret;
	struct TestData test = {
		.test = "Dies ist ein Test",
		.value = true
	};
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (i = 0; i < 1000; i++) {
		ret = buffer_write(buffer3, (uint8_t *) &test, 1);
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		test.value = !test.value;
		vTaskDelayUntil(&lastWakeUpTime, 1 / portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void buffer4Test(void *data) {
	int i;
	int32_t ret;
	struct TestData test;
	bool last = true;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (i = 0; i < 1000; i++) {
		ret = buffer_read(buffer4, (uint8_t *) &test, 1, portMAX_DELAY);
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		printf("recv: %s value: %d len: %ld\n", test.test, test.value, ret);
		if (ret != 1) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		if (last != test.value) {
			printf("Error\n");
			printf("error: %d != %d\n", last, test.value);
			for(;;);
		}
		last = !last;
		vTaskDelayUntil(&lastWakeUpTime, 1 / portTICK_PERIOD_MS);
	}
	printf("Finish Buffer Test\n");
	vTaskDelete(NULL);
}
#endif
#ifdef CONFIG_BUFFERTEST_IRQ
static void bufferIRQTest(void *data) {
	int i;
	int32_t ret;
	char test[] = "Dies ist ein Test";
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (i = 0; i < 100000000000000; i++) {
		ret = buffer_write(bufferTX, (uint8_t *) test, strlen(test) + 1);
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		ret = buffer_read(bufferRX, (uint8_t *) test, 128, portMAX_DELAY);
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		vTaskDelayUntil(&lastWakeUpTime, 1 / portTICK_PERIOD_MS);
	}
}
#endif
#ifdef CONFIG_TTY_BUFFERTEST
static void bufferTTYTest(void *data) {
	int i;
	int32_t ret;
	char test[] = "Dies ist ein Test\n";
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (i = 0; i < 100000000000000; i++) {
		ret = buffer_write(bufferTX, (uint8_t *) test, strlen(test));
		if (ret < 0) {
			printf("error: ret: %ld\n", ret);
			for(;;);
		}
		vTaskDelayUntil(&lastWakeUpTime, 10000 / portTICK_PERIOD_MS);
	}
}
#endif
