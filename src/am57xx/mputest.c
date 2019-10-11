/*
 * Copyright (c) 2016 Andreas Werner <kernel@andy89.org>
 * 
 * Permission is hereby granted, free of charge, to any person 
 * obtaining a copy of this software and associated documentation 
 * files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, 
 * publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS 
 * IN THE SOFTWARE.
 */
#include <stdint.h>
#include <stdio.h>
#include <mpu9250.h>
#include <spi.h>
#include <FreeRTOS.h>
#include <task.h>
#include <devs.h>

void mputest_task(void *data) {
	struct mpu9250 *mpu = data;
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for(;;) {
		struct mpu9250_vector vec;
		int32_t ret;
		ret = mpu9250_getAccel(mpu, &vec, portMAX_DELAY);
		CONFIG_ASSERT(ret >= 0);
		printf("Accel: x: %f y: %f z: %f\n", vec.x, vec.y, vec.z);
		ret = mpu9250_getGyro(mpu, &vec, portMAX_DELAY);
		CONFIG_ASSERT(ret >= 0);
		printf("Gyro: x: %f y: %f z: %f\n", vec.x, vec.y, vec.z);
		vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);
	}
}

MPU9250_ADDDEV(mpu0, 0, 0, SPI_OPT_GPIO_DIS, 500000);

OS_DEFINE_TASK(mpuTask, 1024);
OS_DEFINE_TASK(mpuInitTask, 1024);
void mputest_initTask(void *data) {
	BaseType_t ret;
	struct spi *spi;
	struct mpu9250 *mpu;
	struct accel *accel;
	struct gyro *gyro;
	struct spi_slave *slave[3];
	{
		struct spi_opt opt = {
			.lsb = false,
			.cpol = false,
			.cpha = false,
			.cs = 0,
			.csLowInactive = false,
			.gpio = SPI_OPT_GPIO_DIS,
			.size = 8,
			.wdelay = 0,
			.cs_hold = 8,
			.cs_delay = 500,
			.bautrate = 500000,
		};
		spi = spi_init(SPI1_ID, SPI_3WIRE_CS, NULL);
		CONFIG_ASSERT(spi != NULL);
		/*slave[0] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slave[0] != NULL);*/ /* Init by Driver*/
		opt.cs_hold = 6;
		opt.cs_delay = 8;
		opt.cs = 1;
		slave[1] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slave[1] != NULL);
	}
	printf("MPU9250 init\n");
	do {
		mpu = mpu9250_init(0, portMAX_DELAY);
	} while (mpu != NULL);
	printf("MPU9250 inited\n");
	accel = accel_init(0);
	CONFIG_ASSERT(accel != NULL);
	gyro = gyro_init(0);
	CONFIG_ASSERT(gyro != NULL);
	ret = OS_CREATE_TASK(mputest_task, "MPU Task", 1024, mpu, 1, mpuTask);
	CONFIG_ASSERT(ret == pdPASS);
	vTaskSuspend(NULL);
}

void mputest_init() {
	CONFIG_ASSERT(OS_CREATE_TASK(mputest_initTask, "MPU Init Task", 1024, NULL, 2, mpuInitTask));
}
