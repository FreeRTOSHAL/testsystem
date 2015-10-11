#include <stdio.h>
#include <stdint.h>
#include <adctest.h>
#include <adc.h>
#include <FreeRTOS.h>
#include <task.h>

void adctestTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	struct adc *adc = data;
	int32_t val;
	float vin;
	for (;;) {
		val = adc_get(adc, 0, portMAX_DELAY);
		CONFIG_ASSERT(val >= 0);
		vin = ((float) val) * (3.3 / 4096.) * 10.;
		printf("VIN: %f V\n", vin);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}

int32_t adctest_init() {
	int32_t ret;
	struct adc *adc = adc_init(1, 12, 4125000);
	/*struct adc *adc = adc_init(1, 12, 40000000);*/ /* DIV = 1 */
	/*struct adc *adc = adc_init(1, 12, 13200000);*/ /* DIV = 5 ~ 4 */
	/*struct adc *adc = adc_init(1, 12, 9428571);*/ /* DIV = 7 ~ 8 */
	/*struct adc *adc = adc_init(1, 12, 6000000);*/ /* DIV = 11 ~ 8 */
	/*struct adc *adc = adc_init(1, 12, 4400000);*/ /* DIV = 15 ~ 16 */



	CONFIG_ASSERT(adc != NULL);
	ret = adc_channel(adc, 0);
	CONFIG_ASSERT(ret >= 0);
	xTaskCreate(adctestTask, "ADC Test Task", 512, adc, 1, NULL);
	return 0;
}
