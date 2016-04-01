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


SemaphoreHandle_t sem;
void cpu2cpu_int1_isr(void) {
	BaseType_t xHigherPriorityTaskWoken;
	xSemaphoreGiveFromISR(sem, &xHigherPriorityTaskWoken);
	irq_clear(1);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void pingTask(void *data);
int32_t irqtest_init() {
	sem = xSemaphoreCreateBinary();
	if (sem == NULL) {
		return -1;
	}
	xSemaphoreGive(sem);
	xSemaphoreTake(sem, portMAX_DELAY);
	xTaskCreate( pingTask, "IRQ Pin Test", 512, NULL, 1, NULL);
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
