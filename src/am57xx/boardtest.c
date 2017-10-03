#include <boardtest.h>
#include <gpio.h>
#include <iomux.h>
#include <devs.h>
#include <system.h>
#include <FreeRTOS.h>
#include <task.h>
uint32_t pinIds[] = {
	PAD_XREF_CLK3, /* Servo1 ok */
	PAD_GPMC_A13, /*PAD_MCASP1_AXR15,*/ /* Servo2 ok */
#ifndef CONFIG_TIMER_TEST
	PAD_MCASP1_AXR13, /* EMPF1 ok */
#endif
#ifndef CONFIG_TIMER_TEST
	PAD_MCASP1_AXR14, /* EMPF2 ok */
#endif

	PAD_VIN1A_D19, /* LIGHT1 ok */
	PAD_VIN1A_D22, /* LIGHT2 ok */
	PAD_VIN1A_D12, /* LIGHT3 ok */
	PAD_VIN1A_D14, /* LIGHT4 ok */
	PAD_VIN1A_D4, /* LIGHT5 ok*/
	PAD_VIN1A_D10, /* LIGHT6 ok*/
	PAD_VIN1A_D5, /* LIGHT7 ok*/
	PAD_VIN1A_D16, /* LIGHT8 ok*/
	PAD_VIN1A_D20, /* LIGHT9 ok*/
	PAD_VIN1A_D7, /* LIGHT10 ok*/

	PAD_VIN1A_D8, /* BLED1 ok */
	PAD_VIN1A_D11, /* BLED2 ok */
	PAD_VIN2A_D6, /*PAD_VIN1A_D3,*/ /* BLED3 defekt*/

	PAD_VIN2A_D9, /* BTN1 */
	PAD_VIN2A_DE0, /* BTN2 */
	PAD_VIN2A_CLK0, /* BTN3 */

	PAD_GPMC_A17, /* LED TOP */

	PAD_VIN1A_D2, /* US1_IN defekt*/
	PAD_GPMC_OEN_REN, /* US2_IN ok */
	PAD_XREF_CLK2, /* US3_IN ok */
	PAD_VIN1B_CLK1, /* US4_IN defekt*/

	/*PAD_GPIO6_14,*/
	/*PAD_MMC3_DAT3,*/

	PAD_VIN1A_D3, /*PAD_GPMC_CS2,*/ /* PU1*/
	PAD_MMC3_DAT7, /*PAD_MCASP1_AXR6,*/ /* PU2 */
	PAD_UART3_RXD, /* PU3 */
	PAD_MMC3_CMD, /*PAD_VIN1A_D6,*/ /* PU4 */
	PAD_GPMC_A16, /* PU5 */
	PAD_MCASP1_AXR3 /* PU6 */
};
uint32_t buttonIDs[] = {
	PAD_VIN2A_D9, /* BTN1 */
	PAD_VIN2A_DE0, /* BTN2 */
	PAD_VIN2A_CLK0, /* BTN3 */
	PAD_XREF_CLK1, /* US1_OUT */
	PAD_GPMC_A12, /* US2_OUT */
	PAD_GPIO6_15, /* US3_OUT */
	PAD_GPMC_A15, /* US4_OUT */
	PAD_GPMC_CS2, /* PU1_OUT */
	PAD_MCASP1_AXR6, /* PU2_OUT */
	PAD_UART3_RXD, /* PU3_OUT */
	PAD_VIN1A_D6, /* PU4_OUT */
	PAD_GPMC_A16, /* PU5_OUT */
	PAD_MCASP1_AXR3, /* PU6_OUT */
};
struct gpio_pin *pins[ARRAY_SIZE(pinIds)];
struct gpio_pin *buttons[ARRAY_SIZE(buttonIDs)];


void boardtest_task(void *data) {
	int32_t i;
	int32_t ret;
	(void) data;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (;;) {
		for (i = 0; i < ARRAY_SIZE(pinIds); i++) {
			ret = gpioPin_togglePin(pins[i]);
			CONFIG_ASSERT(ret >= 0);
		}
		vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);
	}
}

static bool buttonHandler(struct gpio_pin *pin, uint32_t pinID, void *data) {
	/* TODO */
	return false;
}
OS_DEFINE_TASK(boardTask, 512);
void boardtest_init() {
	int32_t ret;
	int32_t i;
	struct gpio *gpio = gpio_init(GPIO_ID);
	CONFIG_ASSERT(gpio != NULL);
	for (i = 0; i < ARRAY_SIZE(pinIds); i++) {
		printf("pinID: 0x%lx\n", pinIds[i]);
		pins[i] = gpioPin_init(gpio, pinIds[i], GPIO_OUTPUT, GPIO_PULL_UP);
		CONFIG_ASSERT(pins[i] != NULL);
	}
	for (i = 0; i < ARRAY_SIZE(buttonIDs); i++) {
		buttons[i] = gpioPin_init(gpio, pinIds[i], GPIO_OUTPUT, GPIO_PULL_UP);
		CONFIG_ASSERT(buttons[i] != NULL);
		ret = gpioPin_setCallback(buttons[i], buttonHandler, NULL, GPIO_FALLING);
		CONFIG_ASSERT(ret >= 0);
	}
	ret = OS_CREATE_TASK(boardtest_task, "Board test task", 512, NULL, 1, boardTask);
	CONFIG_ASSERT(ret == pdPASS);
}
