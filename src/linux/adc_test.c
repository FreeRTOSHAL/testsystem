#include <FreeRTOS.h>
#include <task.h>
#include <adc.h>
#include <adc_test.h>
#include <stdio.h>
ADD_ADC_TEST(1);
ADD_ADC_TEST(2);

void adc_task(void *data) {
	int32_t ret;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	struct adc **adcs = data;
	ret = adc_start(adcs[1]);
	CONFIG_ASSERT(ret >= 0);
	for (;;) {
		int32_t value = adc_get(adcs[0], 10 / portTICK_PERIOD_MS);
		printf("adc Value: %d\n", value);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
int32_t adc_callback(struct adc *adc, void *data) {
	static int i = 0;
	return i++ % (((1 << 12) - 1));
}

bool adc_isrCallback(struct adc *adc, uint32_t channel, int32_t value, void *data) {
	printf("adc ISR Value: %d\n", value);
	return false;
}

OS_DEFINE_TASK(task, 500);
void adc_test() {
	int32_t ret;
	static struct adc *adcs[2];
	adcs[0] = adc_init(TEST_ID(1), 12, 1);
	CONFIG_ASSERT(adcs[0]);
	ret = adc_test_connect(adcs[0], adc_callback, NULL);
	CONFIG_ASSERT(ret >= 0);
	adcs[1] = adc_init(TEST_ID(2), 12, 2);
	CONFIG_ASSERT(adcs[1]);
	ret = adc_test_connect(adcs[1], adc_callback, NULL);
	CONFIG_ASSERT(ret >= 0);
	ret = adc_setCallback(adcs[1], adc_isrCallback, NULL);
	CONFIG_ASSERT(ret >= 0);
	OS_CREATE_TASK(adc_task, "ADC Test Task", 500, adcs, 3, task);
}
