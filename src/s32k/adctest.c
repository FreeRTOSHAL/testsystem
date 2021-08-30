#include <adc.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>
#include <s32k_adc.h>

OS_DEFINE_TASK(taskADC, 512);

float adc_val = 0.0;

void adc_task(void *data) {
	struct adc *adc_0_12 = data;
	int32_t adc_0_12_val;
	TickType_t lastWakeUpTime = xTaskGetTickCount();

	for(;;) {
		adc_0_12_val = adc_get(adc_0_12, 1000);
		adc_val = (float)adc_0_12_val/4095.0;
		//printf("adc_0_12: %d %f\n", adc_0_12_val, adc_val);

		vTaskDelayUntil(&lastWakeUpTime, 10 / portTICK_PERIOD_MS);
	}
}


void adc_test() {
	struct adc *adc_0_12 = adc_init(ADC_0_PTC14_ID, 12, 100000);
	CONFIG_ASSERT(adc_0_12);
	adc_set_averaging(adc_0_12, 1);

	OS_CREATE_TASK(adc_task, "ADC Test", 512, adc_0_12, 2, taskADC);
}
