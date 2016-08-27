#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <mailbox.h>
#include <mailbox_test.h>
#include <devs.h>

struct mailbox *rx;
struct mailbox *tx;

static void mailbox_testTask(void *d) {
	int32_t ret;
	uint32_t data;
	for(;;) {
		printf("Recv Data...\n");
		ret = mailbox_recv(rx, &data, portMAX_DELAY);
		CONFIG_ASSERT(ret >= 0);
		printf("Recv Data: 0x%lx\n", data);
		printf("Send Data...\n");
		ret = mailbox_send(tx, data, portMAX_DELAY);
		CONFIG_ASSERT(ret >= 0);
		printf("Data sended\n");
	}
}

void mailbox_test() {
	BaseType_t ret;
	rx = mailbox_init(MAILBOX1_ID);
	CONFIG_ASSERT(rx != NULL);
	tx = mailbox_init(MAILBOX0_ID);
	CONFIG_ASSERT(tx != NULL);
	ret = xTaskCreate(mailbox_testTask, "Mailbox Test Task", 512, NULL, 1, NULL);
	CONFIG_ASSERT(ret == pdPASS);
}

