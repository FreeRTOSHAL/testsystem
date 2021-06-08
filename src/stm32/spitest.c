#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <spi.h>
#include <devs.h>

void spitest_task(void *data) {
	uint16_t send[2];
	uint16_t recv[2];
	struct spi *spi = data;
	struct spi_opt opt = {
		.lsb = true,
		.cpol = true,
		.cpha = false,
		.cs = 0,
		.csLowInactive = false,
		.gpio = SPI_OPT_GPIO_DIS,
		.size = 16,
		.wdelay = 1,
		.cs_hold = 8,
		.cs_delay = 500,
		.bautrate = 328125,
	};
	int ret;
	struct spi_slave *slave = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave);
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	for (;;) {
		send[0] = 0x1234;
		recv[0] = 0x1234;
		send[1] = 0x3456;
		recv[1] = 0x3456;
		ret = spiSlave_transfer(slave, send, recv, 2, 1000 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret >= 0);
		printf("recvData: %d %d\n", recv[0], recv[1]);
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}

OS_DEFINE_TASK(spiTask, 1024);
void spi_test() {
	int ret;
	struct spi *spi = spi_init(SPI2_ID, SPI_3WIRE_CS, NULL);
	CONFIG_ASSERT(spi);
	ret = OS_CREATE_TASK(spitest_task, "SPI test task", 1024, spi, 1, spiTask);
	CONFIG_ASSERT(ret == pdPASS);
}
