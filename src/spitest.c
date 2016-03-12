#include <spitest.h>
#include <FreeRTOS.h>
#include <task.h>
#include <spi.h>
#include <iomux.h>
#include <stdio.h>
#include <system.h>
void spiTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	struct spi_slave **slave = data;
	for (;;) {
		printf("\n\n -- %lu -- \n\n", lastWakeUpTime);
		{
			uint16_t sendData[] = {(0x75 | BIT(7)), 0xFF};
			uint16_t recvData[] = {0x4243, 0x4445};
			int32_t ret;
			printf("MCU Test\n");
			ret = spiSlave_transver(slave[0], sendData, recvData, sizeof(sendData) / sizeof(uint16_t), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: 0x%x 0x%x\n", recvData[0], recvData[1]);
			ret = spiSlave_transver(slave[0], sendData, recvData, sizeof(sendData) / sizeof(uint16_t), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: 0x%x 0x%x\n", recvData[0], recvData[1]);
		}
		/*vTaskDelayUntil(&lastWakeUpTime, 100 / portTICK_PERIOD_MS);*/
		printf("\n\n -- %lu -- \n\n", lastWakeUpTime);
		{
			uint16_t sendData[] = {(0xf | BIT(7)), 0xFF};
			uint16_t recvData[] = {0x4243, 0x4445};
			int32_t ret;
			printf("LSM330DLC Test\n");
			ret = spiSlave_transver(slave[1], sendData, recvData, sizeof(sendData) / sizeof(uint16_t), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: 0x%x 0x%x\n", recvData[0], recvData[1]);
			ret = spiSlave_transver(slave[1], sendData, recvData, sizeof(sendData) / sizeof(uint16_t), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: 0x%x 0x%x\n", recvData[0], recvData[1]);
		}
		printf("\n\n -- %lu -- \n\n", lastWakeUpTime);
		{
			uint16_t sendData[] = {(0xf | BIT(7)), 0xFF};
			uint16_t recvData[] = {0x4243, 0x4445};
			int32_t ret;
			printf("LSM330DLC 2 Test\n");
			ret = spiSlave_transver(slave[2], sendData, recvData, sizeof(sendData) / sizeof(uint16_t), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: 0x%x 0x%x\n", recvData[0], recvData[1]);
			ret = spiSlave_transver(slave[1], sendData, recvData, sizeof(sendData) / sizeof(uint16_t), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: 0x%x 0x%x\n", recvData[0], recvData[1]);
		}
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
	vTaskSuspend(NULL);
}

void spitest_init() {
#if 1
	struct spi *spi;
	static struct spi_slave *slave[3];
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
	slave[0] = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave[0] != NULL);
	opt.cs_hold = 6;
	opt.cs_delay = 8;
	opt.cs = 1;
	slave[1] = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave[1] != NULL);
	opt.cs = 2;
	slave[2] = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave[2] != NULL);
#else
	struct spi *spi;
	static struct spi_slave *slave[3];
	struct spi_ops ops = {
		.lsb = false,
		.cpol = false,
		.cpha = false,
		.cs = SPI_OPT_CS_DIS,
		.csLowInactive = false,
		.gpio = PTD5,
		.size = 8,
		.wdelay = 0,
		.cs_hold = 8,
		.cs_delay = 500,
		.bautrate = 500000,
	};
	spi = spi_init(1);
	CONFIG_ASSERT(spi != NULL);
	ops.cpol = false;
	ops.cpha = false;
	ops.gpio = PTD5; /* CS 0 */
	slave[0] = spi_slave(spi, &ops);
	CONFIG_ASSERT(slave[0] != NULL);
	ops.cpol = false;
	ops.cpha = false;
	ops.cs_hold = 6;
	ops.cs_delay = 8;
	ops.gpio = PTD4; /* CS 1 */
	slave[1] = spi_slave(spi, &ops);
	CONFIG_ASSERT(slave[1] != NULL);
	ops.cpol = false;
	ops.cpha = false;
	ops.gpio = PTD3; /* CS 2 */
	slave[2] = spi_slave(spi, &ops);
	CONFIG_ASSERT(slave[2] != NULL);
#endif

	xTaskCreate(spiTask, "SPI Test Task", 512, slave, 1, NULL);
}
