#include <stdio.h>
#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <event_groups.h> 
#include <timer.h>
#include <pwm.h>
#include <gpio.h>
#if 0
#define WAITTIME (2000 / portTICK_PERIOD_MS)
#define PWMTIME 500000
#define PRESCALER 128
#define BASETIME 500000
#define ADJUST 1120
#else
#define WAITTIME (2000 / portTICK_PERIOD_MS)
#define PWMTIME 1000
#define PRESCALER 4
#define BASETIME 1000
#define ADJUST 203
#endif

#define DUTY_CYCLE (PWMTIME >> 1)

#define GETTIME() *((volatile uint32_t *) 0x40038004)

enum tests {
	SEMAPHORE,
	EVENT,
	NOTIFICATION,
	TASK_SUSPEND,
	END,
};

static struct timer *timer;
static struct pwm *pwm;
static struct gpio *gpio;
static struct gpio_pin *pin;
static TaskHandle_t task;
static SemaphoreHandle_t semaphore;
static EventGroupHandle_t event;
static enum tests acutalTest = SEMAPHORE;
static uint32_t counterValue[END][3];
static float values[END][3];
static inline float counterToUS(uint64_t value) {
	/* Too Many Cast for Optimizer do it step by step */
	float diff;
	float us;
	float v = value;
	float p = PRESCALER;
	float i = (166.736842 / 2.);
	float b = BASETIME;
	diff = b;
	diff += ADJUST;
	
	us = (v * p) / i;
	us = (us * b) / diff;

	return us;
} 



static bool interruptHandler(struct gpio_pin *p, uint8_t pinID, void *data) {
	BaseType_t pxHigherPriorityTaskWoken;
	counterValue[acutalTest][1] = GETTIME();
	switch(acutalTest) {
		case SEMAPHORE:
			xSemaphoreGiveFromISR(semaphore, &pxHigherPriorityTaskWoken);
			break;
		case EVENT:
			xEventGroupSetBitsFromISR(event, BIT(0), &pxHigherPriorityTaskWoken);
			break;
		case NOTIFICATION:
			xTaskNotifyFromISR(task, 0x0, eNoAction, &pxHigherPriorityTaskWoken);
			break;
		case TASK_SUSPEND:
			/* 
			 * Only for Tests: From FreeRTOS Docu:
			 * xTaskResumeFromISR() should not be used to synchronise a task with an interrupt if there is a chance 
			 * that the interrupt could arrive prior to the task being suspended - as this can lead to interrupts being 
			 * missed. Use of a semaphore as a synchronisation mechanism would avoid this eventuality.
			 */
			xTaskResumeFromISR(task); 
			break;
		case END:
			CONFIG_ASSERT(0); /* Interrupt is Called in END State! */
			break;
		default:
			CONFIG_ASSERT(0); /* Interrupt is Called in after END State! */
			break;	
	}
	return pxHigherPriorityTaskWoken;
}

void startTest() {
	pwm_setDutyCycle(pwm, DUTY_CYCLE);
	vTaskDelay(30 / portTICK_PERIOD_MS); /* Ignore First Cycle */
	gpioPin_enableInterrupt(pin);
}
void stopTest() {
	gpioPin_disableInterrupt(pin);
	pwm_setDutyCycle(pwm, 0);
}

void speedtest_task(void *data) {
	BaseType_t ret;
	for(;;) {
		switch(acutalTest) {
			case SEMAPHORE:
				startTest();
				ret = xSemaphoreTake(semaphore, WAITTIME);
				counterValue[acutalTest][2] = GETTIME();
				CONFIG_ASSERT(ret == pdTRUE);
				stopTest();
				break;
			case EVENT:
				startTest();
				ret = xEventGroupWaitBits(event, BIT(0), pdTRUE, pdFALSE, WAITTIME);
				counterValue[acutalTest][2] = GETTIME();
				CONFIG_ASSERT(ret == pdTRUE);
				stopTest();
				break;
			case NOTIFICATION:
				startTest();
				ret = xTaskNotifyWait(0, 0xFFFFFF, NULL, WAITTIME);
				counterValue[acutalTest][2] = GETTIME();
				CONFIG_ASSERT(ret > 0); /* One Bit should set */
				stopTest();
				break;
			case TASK_SUSPEND:
				startTest();
				vTaskSuspend(NULL);
				counterValue[acutalTest][2] = GETTIME();
				stopTest();
				break;
			case END:
				{
					int i;
					int j;
					for (i = 0; i < END; i++) {
						for (j = 0; j < 3; j++) {
							values[i][j] = counterToUS(counterValue[i][j]);
						}
					}
				}
				CONFIG_ASSERT(0); /* End Test */
				break;
			default:
				CONFIG_ASSERT(0); /* Not Defined */
				break;	
		}
		acutalTest++;
	}
}

void speedtest_init() {
	int32_t ret;
	timer = timer_init(0, PRESCALER, BASETIME, ADJUST);
	CONFIG_ASSERT(timer != NULL);

	pwm = pwm_init(0); /* PTB0: FTM0_CH0 */
	CONFIG_ASSERT(pwm != NULL);

	gpio = gpio_init(0);
	CONFIG_ASSERT(gpio != NULL);
	pin = gpioPin_init(gpio, 69, GPIO_INPUT, GPIO_PULL_UP); /* PTD25 */
	CONFIG_ASSERT(pin != NULL);
	ret = gpioPin_setCallback(pin, &interruptHandler, NULL, GPIO_RISING);
	CONFIG_ASSERT(ret == 0);
	ret = pwm_setPeriod(pwm, PWMTIME);
	CONFIG_ASSERT(ret == 0);
	ret = pwm_setDutyCycle(pwm, 0);
	CONFIG_ASSERT(ret == 0);

	xTaskCreate(speedtest_task, "SpeedTestTask", 500, NULL, 2, &task);
	vSemaphoreCreateBinary(semaphore);
	CONFIG_ASSERT(semaphore != NULL);
	xSemaphoreGive(semaphore);
	xSemaphoreTake(semaphore, 0);
	event = xEventGroupCreate();
	CONFIG_ASSERT(event != NULL);
}
