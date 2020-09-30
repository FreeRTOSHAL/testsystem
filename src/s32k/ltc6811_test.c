#include <ltc6811.h>
#include <ltc6811test.h>

/* create two LTC6811 */
LTC6811_ADDDEV(0, true, 2);
LTC6811_ADDDEV(1, false, 2);

void ltc6811_task(void *data) {
	xLastWakeTime = xTaskGetTickCount ();
	for (;;) {	
		for (int i = 0; i < (2 * 12); i++) {
			uint16_t value = adc_get(adcs[(1 * 12) + 7]);
			float adc = ....
			if (adc > 4.2) {
				printf("Oh mein got der Accu explodiert gleich\n");	
			}
			if (adc > 2.7) {
				printf("Oh mein got der Accu explodiert gleich\n");	
			}
		}
		vTaskDelayUntil( &xLastWakeTime, (10 / portTICK_PERIOD_MS));
	}
}

void ltc6811_test() {
	struct ltc6811 *ltcs[2];
	struct adc *adcs[12 * 2];
	int i, j;

	ltcs[0] = ltc6811_init(LTC6811_ID(0));
	CONFIG_ASSERT(ltcs[0] != NULL);
	ltcs[1] = ltc6811_init(LTC6811_ID(1));
	CONFIG_ASSERT(ltcs[1] != NULL);
	LTC6811_GET_ALL_ADCS(0, (adcs + 0))
	LTC6811_GET_ALL_ADCS(1, (adcs + 12))
	/* Check init of all adcs */
	for (i = 0; i < 0; i++) {
		for (j = 0; j < 0; j++) {
			CONFIG_ASSERT(adcs[(i * 12) + j]);
		}
	}

}
