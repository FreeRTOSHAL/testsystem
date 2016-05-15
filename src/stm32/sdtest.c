#include <FreeRTOS.h>
#include <task.h>
#include <devs.h>
#include <sdtest.h>
#include <sd.h>
#include <string.h>
#include <system.h>
struct sd *sd;
uint8_t data[4 * 1024];
uint8_t data2[4 * 1024];

static void testReadWrite() {
	int32_t ret;
	{
		ret = sd_setBlockSize(sd, SD_BLOCK_SIZE_512B);
		CONFIG_ASSERT(ret == 0);
#if 1
		memset(data, 0x42, ARRAY_SIZE(data));
		/* read 4K from card */
		ret = sd_read(sd, CMD(18), 0, ARRAY_SIZE(data), (uint32_t *) data, 1000 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		/* stop transver */
		ret = sd_sendCommand(sd, CMD(12), 0, NULL, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
#else
		memset(data, 0x00, ARRAY_SIZE(data));
#endif
		/* Write Back test */
		ret = sd_write(sd, CMD(25), 0, ARRAY_SIZE(data), (uint32_t *) data, 1000 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		/* stop transver */
		ret = sd_sendCommand(sd, CMD(12), 0, NULL, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		vTaskDelay(500 / portTICK_PERIOD_MS);
		/* read 4K from card */
		ret = sd_read(sd, CMD(18), 0, ARRAY_SIZE(data2), (uint32_t *) data2, 1000 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		/* stop transver */
		ret = sd_sendCommand(sd, CMD(12), 0, NULL, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		ret = memcmp(data, data2, ARRAY_SIZE(data));
		CONFIG_ASSERT(ret == 0);
	}

}

void sdtest_task(void *d) {
	struct sd_response res;
	int32_t ret;
	uint32_t arg;
	uint32_t rca;
	(void) data;
	ret = sd_sendCommand(sd, CMD(0), 0, NULL, 100 / portTICK_PERIOD_MS);
	CONFIG_ASSERT(ret == 0);
	{
		arg = 0;
		arg |= (0x1 << 8); /* Select 2.7 - 3.3 V */
		arg |= (0x42 << 0); /* 8 Bit Check Pattern */
		ret = sd_sendCommand(sd, CMD(8), arg, &res, 100 / portTICK_PERIOD_MS);
		/* Only Version >= 2.0 is suppored in this test */
		CONFIG_ASSERT(ret == 0);
		/* Voltage shall acceped and check pattern is 0x42 */
		CONFIG_ASSERT((res.data[3] & 0xFFF) == ((1 << 8) | (0x42 << 0)));
	}
	{
		arg = 0;
		arg |= BIT(19) | BIT(20) | BIT(31); /* aprox 3.3 is used */;
		arg |= BIT(28); /* Switch from Power Save to max perf */
		arg |= BIT(30); /* Acrivate SDHC or SDXC Support */
		do {
			/* Send ACMD41 -> CMD55 + ACMD41 */
			ret = sd_sendCommand(sd, CMD(55), 0, &res, 100 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			ret = sd_sendCommand(sd, ACMD(41), arg, &res, 100 / portTICK_PERIOD_MS);
			CONFIG_ASSERT(ret == 0);
			
		} while (((res.data[3] >> 31) & 0x1) != 0x1);
	}
	{
		/* Ask for CID */
		ret = sd_sendCommand(sd, CMD(2), 0, &res, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		/* Set Card to ready mode and get RCA*/
		ret = sd_sendCommand(sd, CMD(3), 0, &res, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		rca = res.data[3] & 0xFFFF0000;
		/* Set Card is transver mode */
		ret = sd_sendCommand(sd, CMD(7), rca, 0, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
	}
	testReadWrite();
	{
		arg = 0;
		arg |= 2;
		ret = sd_sendCommand(sd, CMD(55), rca, NULL, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		ret = sd_sendCommand(sd, ACMD(6), arg, NULL, 100 / portTICK_PERIOD_MS);
		CONFIG_ASSERT(ret == 0);
		ret = sd_setBusWide(sd, SD_BusWide_4b);
		CONFIG_ASSERT(ret == 0);
	}
	testReadWrite();
	{
		ret = sd_setClock(sd, 48000000);
		CONFIG_ASSERT(ret == 0);
	}
	testReadWrite();
	vTaskSuspend(NULL);
}
void sdtest_init() {
	struct sd_setting setting = {
		.clock = 400000,
		.wide = SD_BusWide_1b,
		.mode = SD_SD,
	};
	sd = sd_init(SDIO_ID, &setting);
	CONFIG_ASSERT(sd != NULL);

	xTaskCreate(sdtest_task, "SD Test Task", 512, NULL, 1, NULL);
}
