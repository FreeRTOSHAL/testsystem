/* SPDX-License-Identifier: MIT */
/*
 * Author: Andreas Werner <kernel@andy89.org>
 * Date: 2016
 */
#include <stdint.h>
#include <stdio.h>
#include <mpu9250.h>
#include <spi.h>
#include <FreeRTOS.h>
#include <task.h>

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

MPU9250_ADDDEV(mpu0, 1, 0, SPI_OPT_GPIO_DIS, 500000);
OS_DEFINE_TASK(mpuTask, 512);
void mputest_initTask(void *data) {
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
		spi = spi_init(1, SPI_3WIRE_CS, NULL);
		CONFIG_ASSERT(spi != NULL);
		/*slave[0] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slave[0] != NULL);*/ /* Init by Driver*/
		opt.cs_hold = 6;
		opt.cs_delay = 8;
		opt.cs = 1;
		slave[1] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slave[1] != NULL);
		opt.cs = 2;
		slave[2] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slave[2] != NULL);
	}
	mpu = mpu9250_init(0, portMAX_DELAY);
	CONFIG_ASSERT(mpu != NULL);
	accel = accel_init(0);
	CONFIG_ASSERT(accel != NULL);
	gyro = gyro_init(0);
	CONFIG_ASSERT(gyro != NULL);
	OS_CREATE_TASK(mputest_task, "MPU Task", 1024, mpu, 1, mpuTask);
	vTaskSuspend(NULL);
}

OS_DEFINE_TASK(mpuInitTask, 512);
void mputest_init() {
	CONFIG_ASSERT(OS_CREATE_TASK(mputest_initTask, "MPU Init Task", 512, NULL, 2, mpuInitTask));
}
