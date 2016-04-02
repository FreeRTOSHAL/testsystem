#include <stdio.h>
#include <stdlib.h>
#include <timer.h>
#include <devs.h>

struct timer *tim1;

static bool timertest_callback(struct timer *timer, void *data) {
	printf("Timer Overflow\n");
	return false;
}

void timertest_init() {
	int32_t ret;
	tim1 = timer_init(TIMER1_ID, 2000, 1, 0);
	CONFIG_ASSERT(tim1 != NULL);
	ret = timer_setOverflowCallback(tim1, timertest_callback, NULL);
	CONFIG_ASSERT(ret == 0);
	/*ret = timer_periodic(tim1, 1ULL * 1000ULL * 1000ULL);
	CONFIG_ASSERT(ret == 0);*/
	ret = timer_periodic(tim1, 100ULL * 1000ULL);
	CONFIG_ASSERT(ret == 0);
}
