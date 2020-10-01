#include <ltc6811.h>
#include <ltc6811test.h>

/* create two LTC6811 */
LTC6811_ADDDEV(0, 2);
LTC6811_SLAVE_ADDDEV(0, 0);
LTC6811_SLAVE_ADDDEV(0, 1);

void ltc6811_task(void *data) {
	struct adc **adcs = data;
	TickType_t xLastWakeTime = xTaskGetTickCount ();
	for (;;) {	
		for (int i = 0; i < (2 * 12); i++) {
			uint16_t value = adc_get(adcs[(1 * 12) + 7], 0);
			#define MAGIC 0 /* TODO ^^ */
			float adc = value * MAGIC;
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
	struct ltc6811 *ltcs;
	struct ltc6811_slave *slaves[2];
	struct adc *adcs[12 * 2];
	int i, j;

	ltcs = ltc6811_init(LTC6811_ID(0));
	CONFIG_ASSERT(ltcs != NULL);
	slaves[0] = ltc6811_slave_init(LTC6811_SLAVE_ID(0, 0));
	CONFIG_ASSERT(slaves[0] != NULL);
	slaves[1] = ltc6811_slave_init(LTC6811_SLAVE_ID(0, 1));
	CONFIG_ASSERT(slaves[1] != NULL);
	LTC6811_GET_ALL_ADCS(0, 0, (adcs + 0))
	LTC6811_GET_ALL_ADCS(0, 1, (adcs + 12))
	/* Check init of all adcs */
	for (i = 0; i < 0; i++) {
		for (j = 0; j < 0; j++) {
			CONFIG_ASSERT(adcs[(i * 12) + j]);
		}
	}

}
