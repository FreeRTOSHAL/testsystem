#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <stdio.h>

void floatTest(void *d) {
	uint32_t *id = d;
	register float test = 0.0;
	register float inc = ((float) *id) / 100.;
	for(;;) {
		portYIELD();
		test += inc;
		portYIELD();
		printf("ID: %u %f\n", *id, test);
	}
}

OS_DEFINE_TASK(testTask, 1024);
OS_DEFINE_TASK(testTask2, 1024);
void floattest_init() {
	BaseType_t bret;
	static uint32_t opt[2] = {1,2};
	bret = OS_CREATE_TASK(floatTest, "FloatTest", 1024, &opt[0], 2, testTask);
	CONFIG_ASSERT(bret == pdPASS);
	bret = OS_CREATE_TASK(floatTest, "FloatTest2", 1024, &opt[1], 2, testTask2);
	CONFIG_ASSERT(bret == pdPASS);
}
