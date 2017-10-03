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
#include <FreeRTOS.h>
#include <task.h>
#include <linux_client.h>
#include <stdio.h>
#include <vector.h>
#include <string.h>
#include <irq.h>
#include <buffer.h>
#include <os.h>

struct lc {
	bool init;
	struct motor *motor;
	struct buffer *rx;
	struct buffer *tx;
	void (*callbacks[LC_COUNT])(struct lc *lc, struct lc_msg *msg);
};
#define BUFFER_RX ((struct buffer_base *) 0x3F07DC38)
#define BUFFER_TX ((struct buffer_base *) 0x3F07DE50)
struct lc lc0;
static void lc_shutdown(struct lc *lc) {
	vTaskSuspendAll(); /* Stop all Tasks */
	taskDISABLE_INTERRUPTS(); /* Disable all Interrupts */
}
void lcTask(void *data);
void lc_ping(struct lc *lc, struct lc_msg *msg) {
	int32_t ret;
	ret = lc_send(lc, msg);
	if (ret < 0) {
		printf("Can't not Send");
	}
}
OS_DEFINE_TASK(LCTask, 500);
struct lc *lc_init(struct motor *motor) {
	struct lc *lc = &lc0;
	if (lc->init) {
		return lc;
	}
	memset(lc, 0, sizeof(struct lc));
	lc->motor = motor;
	lc->rx = buffer_init(BUFFER_RX, 8, 32, true, 2);
	if (lc->rx == NULL) {
		return NULL;
	}
	lc->tx = buffer_init(BUFFER_TX, 8, 32, false, 2);
	if (lc->tx == NULL) {
		return NULL;
	}
#ifdef CONFIG_MACH_VF610
	irq_clear(3);
	irq_setPrio(3, 0xFF);
	irq_enable(3);
	irq_clear(0);
	irq_setPrio(0, 0xFF);
	irq_enable(0);
#endif
	OS_CREATE_TASK(lcTask, "Linux Client", 500, lc, 2, LCTask);
	lc->init = true;
	lc_registerCallback(lc, LC_TYPE_ACT, lc_ping);
	return lc;
}
int32_t lc_registerCallback(struct lc *lc, uint8_t type, void (*callback)(struct lc *lc, struct lc_msg *msg)) {
	lc->callbacks[type] = callback;
	return 0;
}
int32_t lc_send(struct lc *lc, struct lc_msg *msg) {
	return buffer_write(lc->tx, (uint8_t *) msg, 1);
}
int32_t lc_sendAct(struct lc *lc) {
	struct lc_msg msg= {.type = LC_TYPE_ACT};
	return lc_send(lc, &msg);
}
void lcTask(void *data) {
	struct lc *lc = data;
	struct lc_msg msg;
	int32_t ret;
	for(;;) {
		ret = buffer_read(lc->rx, (uint8_t *) &msg, 1, portMAX_DELAY);
		if (ret < 0) {
			printf("Recv Buffer RX Error: %ld\n", ret);
			/* TODO Error */
			continue;
		}
		if (msg.type < LC_COUNT) {
			if (lc->callbacks[msg.type] != NULL) {
				lc->callbacks[msg.type](lc, &msg);
			}
		} else {
			printf("Recv Unkown Type: %d\n", msg.type);
			/* TODO Error */
			continue;
		}
	}
}
int32_t lc_deinit(struct lc *lc) {
	vPortFree(lc);
	return 0;
}
#ifdef CONFIG_MACH_VF610
void cpu2cpu_int3_isr(void) {
	/* Emergency Shutdown */
	irq_clear(3);
	lc_shutdown(&lc0);
	for(;;); /* Block in Intterrupt */
}
#if 0
uint32_t *AIRCR = (uint32_t *) 0xE000ED0C;
void (*bootloader)() = (void *) 0x1f03ec01;
void cpu2cpu_int0_isr(void) {
	/* jump Bootloader */
	irq_clear(0);
	lc_shutdown(&lc0);
	*AIRCR |= (0x1 << 1);
	bootloader();
	for(;;); /* Never reach */
}
#endif
#ifdef CONFIG_FAKE_BOOTLOADER
/* 
 * For Jump to Bootlaoder out of ISR, we use this Fake Stack
 * The local Reset Register and Externel Reset Register will not work on VF610 
 * Also the Exeption Reset Register
 */
uint32_t  booloaderFakeStack[] = {
	[0] = 0x42424242, /* R0 */
	[1] = 0x42424242, /* R1 */
	[2] = 0x42424242, /* R2 */
	[3] = 0x42424242, /* R3 */
	[4] = 0x42424242, /* R12 */
	[5] = 0x1f03ec01, /* LR Register set to Bootloader Entry Point */
	[6] = 0x1f03ec01, /* Bootloader PC */
	[7] = 0x01000000, /* XPSR */
};
__attribute__((naked)) void cpu2cpu_int0_isr(void) {
	asm volatile (
		"cpsid i" "\n"
		"cpsid f" "\n"
		"push {r4-r12, lr}"
	);
	/* Shutdown all PWM */
	irq_clear(0);
	lc_shutdown(&lc0);
	/* jump Bootloader */
	asm volatile (
		"pop {r4-r12, lr}" "\n"
		"ldr r2, =booloaderFakeStack" "\n"
		"msr psp, r2" "\n"
		"isb" "\n"
		"msr msp, r2" "\n"
		"isb" "\n"
		"bx lr" "\n"
	);
}
#else
#include <core_cm4.h>
void cpu2cpu_int0_isr(void) {
	irq_clear(0);
	/* Shutdown all PWM */
	lc_shutdown(&lc0);
	__DSB();
	SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos) | // Key for reset
		(SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
		SCB_AIRCR_VECTRESET_Msk); // Call local reset, boot into ROM -> ROM Boot Bootloader -> Bootloader
	__DSB();
 	for(;;); // Should never reach
}
#endif
#endif
