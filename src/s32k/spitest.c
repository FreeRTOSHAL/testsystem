#include <stdio.h>
#include <string.h>
#include <spi.h>
#include <devs.h>
#include <sd.h>
#include <iomux.h>


OS_DEFINE_TASK(taskSPI, 512);

/*
 * Based on http://en.wikipedia.org/wiki/Computation_of_CRC
 */
static uint8_t crc7(uint16_t *data, size_t len)
{
        uint8_t e, f, crc;
        size_t i;

        crc = 0;
        for (i = 0; i < len; i++) {
                e   = crc ^ ((uint8_t) data[i]);
                f   = e ^ (e >> 4) ^ (e >> 7);
                crc = (f << 1) ^ (f << 4);
        }
        return crc >> 1;
}
#define SD_COMMAND_OFFSET (1)
#define SD_COMMAND_LEN (SD_COMMAND_OFFSET /* idle bytes */ + 1 /* index */ + 4 /* Argument */ + 1 /* CRC */ + 8 /* NCR (0-8 bytes) */ + 1 /* Response */ + 8 /* idle bytes */)

int32_t sendCommand(struct spi_slave *slave, int8_t cmd, int32_t arg) {
	int i;
	int32_t ret;
	uint32_t len = SD_COMMAND_LEN;
	uint16_t send[SD_COMMAND_LEN];
	uint16_t recv[SD_COMMAND_LEN];
	memset(send, 0xFF, len * sizeof(uint16_t)); /* fill all bytes with 0xFF */
	memset(recv, 0xFF, len * sizeof(uint16_t)); /* fill all bytes with 0xFF */
	send[SD_COMMAND_OFFSET + 0] = BIT(6) /* bit 6 is set every time */ | CMD(0);
	send[SD_COMMAND_OFFSET + 1] = (arg >> 24) & 0xFF;
	send[SD_COMMAND_OFFSET + 2] = (arg >> 16) & 0xFF;
	send[SD_COMMAND_OFFSET + 3] = (arg >> 8) & 0xFF;
	send[SD_COMMAND_OFFSET + 4] = arg & 0xFF;
	send[SD_COMMAND_OFFSET + 5] = (crc7((send + SD_COMMAND_OFFSET), 5) << 1) | BIT(0) /* bit 0 is set every time */;
	ret = spiSlave_transver(slave, send, recv, len, 1000 / portTICK_PERIOD_MS);
	CONFIG_ASSERT(ret >= 0);
	printf("send: ");
	for (i = 0; i < len; i++) {
		printf("0x%x ", send[i]);
	}
	printf("\n");
	printf("recv: ");
	for (i = SD_COMMAND_OFFSET + 5; i < len; i++) {
		printf("0x%x ", recv[i]);
	}
	printf("\n");
	return 0;
}

void spiTask(void *s) {
	struct spi_slave **slaves = s;
	int  i;
	for (i = 0; i < 2; i++) {
	/*
	 * SD Reset test
	 */
	{
		{
			int32_t ret;
			uint32_t len = (100/8);
			uint16_t recv[100/8];
			/* can be smaller, a SD card need 74 clock pulses to start up */
			memset(recv, 0xFF, len * sizeof(uint16_t)); /* fill all bytes with 0xFF */
			ret = spiSlave_recv(slaves[0], recv, len, 1000 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
		}
		sendCommand(slaves[0], CMD(0), 0);
	}
	}
	return;
	/* 
	 * RTC Test
	 */
	{
		#define RTC_MSG_LEN (1 + 3)
		int32_t ret;
		uint16_t len = RTC_MSG_LEN;
		uint16_t send[RTC_MSG_LEN];
		uint16_t recv[RTC_MSG_LEN];
		int i;
		printf("Read Contoll bytes from RTC\n");
		/* try to read Contoll the 3 bytes */
		memset(send, 0xFF, len * sizeof(uint16_t)); /* fill all bytes with 0xFF */
		memset(recv, 0xFF, len * sizeof(uint16_t)); /* fill all bytes with 0xFF */
		send[0] = BIT(7) /* read */ | BIT(5) /* Is every time set */ | 0 /* start with address 0 */;
		ret = spiSlave_transver(slaves[2], send, recv, len, 1000 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret >= 0);
		printf("send: ");
		for (i = 0; i < len; i++) {
			printf("0x%x ", send[i]);
		}
		printf("\n");
		printf("recv: ");
		for (i = 0; i < len; i++) {
			printf("0x%x ", recv[i]);
		}
	}
}

static struct spi_slave *slaves[3];
void spitest_init() {
	struct spi *spi = spi_init(LPSPI0_ID, SPI_3WIRE_CS, NULL);
	{
		struct spi_opt opt = {
			.lsb = false,
			.cpol = true,
			.cpha = true,
			.cs = 0,
			//.cs = SPI_OPT_CS_DIS,
			.csLowInactive = false,
			.gpio = SPI_OPT_GPIO_DIS,
			//.gpio = PTB5,
			//.gpio = PTB0,
			.size = 8,
			.wdelay = 8,
			.cs_hold = 8,
			.cs_delay = 100, 
			.bautrate = 100000,
		};
		/* MMC / SD */
		slaves[0] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slaves[0]);
	}
#if 0
	spi = spi_init(LPSPI1_ID, SPI_3WIRE_CS, NULL);
	{
		struct spi_opt opt = {
			.lsb = false,
			.cpol = false,
			.cpha = false,
			.cs = 0,
			//.cs = SPI_OPT_CS_DIS,
			.csLowInactive = false,
			.gpio = SPI_OPT_GPIO_DIS,
			//.gpio = PTE1,
			.size = 8,
			.wdelay = 8,
			.cs_hold = 8,
			.cs_delay = 100, 
			.bautrate = 100000,
		};
		/* LTC */
		slaves[1] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slaves[1]);
	}
	{
		struct spi_opt opt = {
			.lsb = false,
			.cpol = false,
			.cpha = false,
			.cs = 1,
			//.cs = SPI_OPT_CS_DIS,
			.csLowInactive = false,
			.gpio = SPI_OPT_GPIO_DIS,
			//.gpio = PTA6,
			.size = 8,
			.wdelay = 8,
			.cs_hold = 8,
			.cs_delay = 100,
			.bautrate = 100000,
		};
		/* RTC */
		slaves[2] = spiSlave_init(spi, &opt);
		CONFIG_ASSERT(slaves[2]);
	}
#endif
	OS_CREATE_TASK(spiTask, "SPI Test", 512, slaves, 2, taskSPI);
}
