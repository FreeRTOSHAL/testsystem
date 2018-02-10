#include <boardtest.h>
#include <gpio.h>
#include <iomux.h>
#include <devs.h>
#include <system.h>
#include <FreeRTOS.h>
#include <task.h>
#include <stdint.h>
#include <stdio.h>

struct pinDesc {
	char *name;
	uint32_t pinID;
};

#define PIN(name, pinID) {name, pinID}


/* 
 * TIM10
 * TIM11
 * TIM12
 * TIM13
 * TIM14
 */

struct pinDesc pinIds[] = {
	PIN("SERVO_OUT", PTB8), /* +TIM4_CH3 TIM10_CH1 */
	PIN("SERVO_IN", PTB4), /* TIM3_CH1, JTAG NJTRST */
	PIN("DRIVE OUT", PTE5), /* TIM9_CH1 */
	//PIN("DRIVE IN", PTB3), /* SWO TIM2_CH2 */
	PIN("DRIVE IN", PTE6), /* TIM9_CH2 */

	PIN("RSHR", PTB5), /* TIM5_CH4 */
	//PIN("RSHL", PTD2), /* TIM3_ETR ??? GPIO PTD2 */ 
	PIN("RSHL", PTE13), /* TIM2_CH3 */ 
	PIN("RSVR", PTA3), /* +TIM2_CH4 TIM5_CH4 TIM9_CH2 */
	//PIN("RSVL", PTB1), /* TIM3_CH4 TIM8_CH3N TIM1_CH3N -> No Timer */
	PIN("RSVL", PTA1), /* TIM5_CH3 */

	PIN("USS 1", PTA8), /* TIM1_CH1 */
	PIN("USS 2", PTC9), /* +TIM8_CH4 TIM3_CH4 */

	PIN("SPI SCK", PTB13), /* SPI2_SCK */
	PIN("SPI MOSI", PTB15), /* SPI2_MOSI */
	PIN("SPI INT", PTD9), /* GPIO IN PTD9 */
	PIN("SPI NSS", PTB12), /* SPI_NSS */
	PIN("SPI MISO", PTB14), /* SPI2_MISO */

	PIN("UART TX", PTC6), /* USART6_TX */
	PIN("UART RX", PTC7), /* USART6_RX */

	PIN("LAMPS 1", PTC8), /* GPIO OUT PTC8 */
	PIN("LAMPS 2", PTD0), /* GPIO OUT PTD0 */
	PIN("LAMPS 3", PTC11), /* GPIO OUT PTC11 */
	/*
	 * PC13, PC14, PC15 and PI8 are supplied through the power switch. Since the switch only sinks a limited amount of current
	 (3 mA), the use of GPIOs PC13 to PC15 and PI8 in output mode is limited:
	 - The speed should not exceed 2 MHz with a maximum load of 30 pF.
	 - These I/Os must not be used as a current source (e.g. to drive an LED).
	 *
	 *  Main function after the first backup domain power-up. Later on, it depends on the contents of the RTC registers even after
	 *  reset (because these registers are not reset by the main reset). For details on how to manage these I/Os, refer to the RTC
	 *  register description sections in the STM32F4xx reference manual, available from the STMicroelectronics website:
	 *  www.st.com.
	 */
	PIN("LAMPS RC", PTC13),/* GPIO OUT PTC13 */ 
	PIN("LAMPS STATUS 1", PTE10), /* GPIO OUT PTE10 */
	PIN("LAMPS STATUS 2", PTE12), /* GPIO OUT PTE12 */
	PIN("LAMPS STATUS 3", PTE14), /* GPIO OUT PTE14 */
};
struct pinDesc buttonIDs[] = {
	PIN("DISTS", PTA15), /* GPIO IN PA15, JTDI */
	PIN("BTN STAUS 1", PTD1), /* GPIO IN PTD1 */
	PIN("BTN STAUS 2", PTD3), /* GPIO IN PTD3 */
	PIN("BTN STAUS 3", PTD7), /* GPIO IN PTD7 */
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
	int i;
	printf("Button pressed: %lu ", pinID);
	for (i = 0; i < ARRAY_SIZE(buttonIDs); i++) {
		if (pinID == buttonIDs[i].pinID) {
			printf("%s", buttonIDs[i].name);
		}
	}
	printf("\n");
	return false;
}
OS_DEFINE_TASK(boardTask, 512);
void boardtest_init() {
	int32_t ret;
	int32_t i;
	struct gpio *gpio = gpio_init(GPIO_ID);
	CONFIG_ASSERT(gpio != NULL);
	for (i = 0; i < ARRAY_SIZE(pinIds); i++) {
		printf("pinID: %s 0x%lx\n", pinIds[i].name, pinIds[i].pinID);
		pins[i] = gpioPin_init(gpio, pinIds[i].pinID, GPIO_OUTPUT, GPIO_PULL_UP);
		CONFIG_ASSERT(pins[i] != NULL);
	}
	for (i = 0; i < ARRAY_SIZE(buttonIDs); i++) {
		buttons[i] = gpioPin_init(gpio, buttonIDs[i].pinID, GPIO_INPUT, GPIO_OPEN);
		CONFIG_ASSERT(buttons[i] != NULL);
		ret = gpioPin_setCallback(buttons[i], buttonHandler, NULL, GPIO_RISING);
		CONFIG_ASSERT(ret >= 0);
		ret = gpioPin_enableInterrupt(buttons[i]);
		CONFIG_ASSERT(ret >= 0);
	}
	ret = OS_CREATE_TASK(boardtest_task, "Board test task", 512, NULL, 1, boardTask);
	CONFIG_ASSERT(ret == pdPASS);
}
