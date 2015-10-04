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

void mputest_initTask(void *data) {
	struct spi *spi;
	struct mpu9250 *mpu;
	struct spi_slave *slave[3];
	spi = spi_init(1);
	CONFIG_ASSERT(spi != NULL);
	{
		struct spi_ops ops = {
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
		spi = spi_init(1);
		CONFIG_ASSERT(spi != NULL);
		slave[0] = spi_slave(spi, &ops);
		CONFIG_ASSERT(slave[0] != NULL);
		ops.cs_hold = 6;
		ops.cs_delay = 8;
		ops.cs = 1;
		slave[1] = spi_slave(spi, &ops);
		CONFIG_ASSERT(slave[1] != NULL);
		ops.cs = 2;
		slave[2] = spi_slave(spi, &ops);
		CONFIG_ASSERT(slave[2] != NULL);
	}
	mpu = mpu9250_init(slave[0], portMAX_DELAY);
	CONFIG_ASSERT(mpu != NULL);
	xTaskCreate(mputest_task, "MPU Task", 1024, mpu, 1, NULL);
	vTaskSuspend(NULL);
}

void mputest_init() {
	CONFIG_ASSERT(xTaskCreate(mputest_initTask, "MPU Init Task", 512, NULL, 2, NULL));
}
