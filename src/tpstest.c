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
	for(;;) {
		val = tps_diag(tps, TPS_VDD5, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VDD5: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VDD6, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VDD6: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VCP, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VCP: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VSOUT1, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VSOUT1: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VBAT_SAFING, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VBAT_SAFING: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VBAT, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VBAT: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_MAIN_BG, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_MAIN_BG: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
		val = tps_diag(tps, TPS_VMON_BG, adc, 1, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(val != NAN);
		printf("TPS_VMON_BG: %f\n", val);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}

static void tpstest_initTask(void *data) {
	struct spi *spi;
	struct spi_ops ops = {
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
	int32_t ret;
	spi = spi_init(0);
	CONFIG_ASSERT(spi != NULL);
	slave = spi_slave(spi, &ops);
	CONFIG_ASSERT(slave != NULL);
	tps = tps_init(slave, 100 / portTICK_PERIOD_MS);
	CONFIG_ASSERT(tps != NULL);
	adc = adc_init(1, 12, 4125000);
	CONFIG_ASSERT(adc != NULL);
	ret = adc_channel(adc, 1);
	CONFIG_ASSERT(ret >= 0);
	xTaskCreate(tpstest_task, "TPS Task", 512, NULL, 1, NULL);
	vTaskSuspend(NULL);
}

int32_t tpstest_init() {
	xTaskCreate(tpstest_initTask, "TPS Init Task", 512, NULL, 4, NULL);
	return 0;
}
