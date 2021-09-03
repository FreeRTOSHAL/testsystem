#include <uart.h>
#include <devs.h>
#include <sbi_uart.h>

static void echoTask(void *data) {
	struct uart *uart = data;
	char c;
	int32_t ret;
	uart_puts(uart, "Start Testing\n\r", portMAX_DELAY);
	for (;;) {
		c = uart_getc(uart, portMAX_DELAY);
		//c = uart_getcISR(uart);
		if (c == '\r') {
			ret = uart_putc(uart, '\n', portMAX_DELAY);
			CONFIG_ASSERT(ret >= 0);
		}
		ret = uart_putc(uart, c, portMAX_DELAY);
		CONFIG_ASSERT(ret >= 0);
	}
}

OS_DEFINE_TASK(echo, 512);
void uartTest_init() {
	BaseType_t bret;
#ifdef CONFIG_UART_TEST_SBI
	struct uart *uart = uart_init(SBI_UART_ID, 115200);
#else
	struct uart *uart = uart_init(UART0_ID, 115200);
#endif
	CONFIG_ASSERT(uart != NULL);
	bret = OS_CREATE_TASK(echoTask, "echo", 512, uart, 2, echo);
	CONFIG_ASSERT(bret == pdPASS);
}
