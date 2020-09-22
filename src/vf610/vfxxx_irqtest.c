/* SPDX-License-Identifier: MIT */
/*
 * Author: Andreas Werner <kernel@andy89.org>
 * Date: 2016
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <gpio.h>
#include <iomux.h>
#include <uart.h>
#include <newlib_stub.h>
#include <buffertest.h>
#include <irq.h>
#include <vector.h>
#include <semphr.h>
#include <os.h>


OS_DEFINE_SEMARPHORE_BINARAY(sem);
void cpu2cpu_int1_isr(void) {
	BaseType_t xHigherPriorityTaskWoken;
	xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
	irq_clear(1);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void pingTask(void *data);
OS_DEFINE_TASK(taskPing, 512);
int32_t irqtest_init() {
	sem = OS_CREATE_SEMARPHORE_BINARAY(sem);
	if (sem == NULL) {
		return -1;
	}
	xSemaphoreGive(sem);
	xSemaphoreTake(sem, portMAX_DELAY);
	OS_CREATE_TASK( pingTask, "IRQ Pin Test", 512, NULL, 1, taskPing);
	return 0;
}
void pingTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	irq_clear(1);
	irq_setPrio(1, 0xFF);
	irq_enable(1);
	for(;;) {
		irq_notify(0, 1);
		xSemaphoreTake(sem, 10000 / portTICK_PERIOD_MS);
		vTaskDelayUntil(&lastWakeUpTime, 10000 / portTICK_PERIOD_MS);		
	}
}
