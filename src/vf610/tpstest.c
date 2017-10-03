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
#include <stdint.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <math.h>
#include <tps65381.h>
static struct adc *adc;
static struct tps65381 *tps;

static void tpstest_task(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	float val;
	int32_t ret;
	for(;;) {
		ret = tps_mux(tps, TPS_VDD5, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VDD5, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VDD5: %f V\n", val);

		ret = tps_mux(tps, TPS_VDD6, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VDD6, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VDD6: %f V\n", val);

		ret = tps_mux(tps, TPS_VCP, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VCP, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VCP: %f V\n", val);

		ret = tps_mux(tps, TPS_VSOUT1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VSOUT1, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VSOUT1: %f V\n", val);

		ret = tps_mux(tps, TPS_VBAT_SAFING, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VBAT_SAFING, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VBAT_SAFING: %f V\n", val);

		ret = tps_mux(tps, TPS_VBAT, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VBAT, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VBAT : %f V\n", val);

		ret = tps_mux(tps, TPS_MAIN_BG, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_MAIN_BG, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_MAIN_BG: %f V\n", val);

		ret = tps_mux(tps, TPS_VMON_BG, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VMON_BG, adc, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VMON_BG: %f V\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
OS_DEFINE_TASK(tpsTask, 512);
static void tpstest_initTask(void *data) {
	struct spi *spi;
	struct spi_opt opt = {
		.lsb = false,
		.cpol = false,
		.cpha = true,
		.cs = 0,
		.csLowInactive = false,
		.gpio = SPI_OPT_GPIO_DIS,
		.size = 8, 
		.wdelay = 0,
		.cs_hold = 54,
		.cs_delay = 54,
		.bautrate = 500000
	};
	struct spi_slave *slave;
	spi = spi_init(0, SPI_3WIRE_CS, NULL);
	CONFIG_ASSERT(spi != NULL);
	slave = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave != NULL);
	tps = tps_init(slave, 100 / portTICK_PERIOD_MS);
	CONFIG_ASSERT(tps != NULL);
	adc = adc_init(1, 12, 4125000);
	CONFIG_ASSERT(adc != NULL);
	OS_CREATE_TASK(tpstest_task, "TPS Task", 512, NULL, 1, tpsTask);
	vTaskSuspend(NULL);
}
OS_DEFINE_TASK(tpsInitTask, 512);
int32_t tpstest_init() {
	OS_CREATE_TASK(tpstest_initTask, "TPS Init Task", 512, NULL, 4, tpsInitTask);
	return 0;
}
