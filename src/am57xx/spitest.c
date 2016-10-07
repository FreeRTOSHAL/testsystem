#include <spitest.h>
#include <FreeRTOS.h>
#include <task.h>
#include <spi.h>
#include <devs.h>
#include <stdio.h>
#include <system.h>
#include <string.h>

void printHex(uint16_t *data, size_t len) {
	size_t i;
	printf(", 0x%x", data[0]);
	for (i = 1; i < len; i++) {
		printf(", 0x%x", data[i]);
		if ((i % 16) == 0) {
			printf("\n");
		}
	}
	printf("\n");
}

void spiTask(void *data) {
	TickType_t lastWakeUpTime = xTaskGetTickCount();
	struct spi_slave **slave = data;
	for (;;) {
		printf("\n\n -- %lu -- \n\n", lastWakeUpTime);
		{
			int32_t ret;
			uint16_t sendData[] = {(0x75 | BIT(7)), 0xFF};
			uint16_t recvData[] = {0x4243, 0x4445};
			printf("MPU Test\n");
			ret = spiSlave_transver(slave[0], sendData, recvData, ARRAY_SIZE(sendData), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: ");
			printHex(recvData, ARRAY_SIZE(recvData));
			ret = spiSlave_transver(slave[0], sendData, recvData, ARRAY_SIZE(sendData), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: ");
			printHex(recvData, ARRAY_SIZE(recvData));
		}
		{
			int32_t ret;
			uint16_t sendData[32];
			uint16_t recvData[ARRAY_SIZE(sendData)];
			memset(sendData, 0xFF, sizeof(sendData) * sizeof(uint16_t));
			memset(sendData, 0xFF, sizeof(sendData) * sizeof(uint16_t));
			printf("ADC Test len: %d\n", ARRAY_SIZE(sendData));
			ret = spiSlave_transver(slave[1], sendData, recvData, ARRAY_SIZE(sendData), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: ");
			printHex(recvData, ARRAY_SIZE(recvData));
			ret = spiSlave_transver(slave[1], sendData, recvData, ARRAY_SIZE(sendData), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: ");
			printHex(recvData, ARRAY_SIZE(recvData));
		}
		{
			int32_t ret;
			uint16_t sendData[32];
			uint16_t recvData[ARRAY_SIZE(sendData)];
			memset(sendData, 0xFF, sizeof(sendData) * sizeof(uint16_t));
			memset(sendData, 0xFF, sizeof(sendData) * sizeof(uint16_t));
			printf("ADC Test len: %d\n", ARRAY_SIZE(sendData));
			ret = spiSlave_transver(slave[2], sendData, recvData, ARRAY_SIZE(sendData), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: ");
			printHex(recvData, ARRAY_SIZE(recvData));
			ret = spiSlave_transver(slave[2], sendData, recvData, ARRAY_SIZE(sendData), 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			printf("recv: ");
			printHex(recvData, ARRAY_SIZE(recvData));
		}
		vTaskDelayUntil(&lastWakeUpTime, 1000 / portTICK_PERIOD_MS);
	}
}
void spitest_init() {
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
		.bautrate = 500000
	};
	printf("Init SPI\n");
	spi = spi_init(SPI4_ID, SPI_3WIRE_CS, NULL);
	CONFIG_ASSERT(spi != NULL);
	printf("Init SPI Slave 0\n");
	slave[0] = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave[0] != NULL);
	opt.cs_hold = 6;
	opt.cs_delay = 8;
	opt.cs = 1;
	opt.size = 16;
	printf("Init SPI Slave 1\n");
	slave[1] = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave[1] != NULL);
	opt.cs = 3;
	printf("Init SPI Slave 2\n");
	slave[2] = spiSlave_init(spi, &opt);
	CONFIG_ASSERT(slave[2] != NULL);
	xTaskCreate(spiTask, "SPI Test Task", 512, slave, 1, NULL);
}
