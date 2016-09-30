#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <system.h>
#include <net/phy.h>
#include <net/phydev.h>
#include <MCIMX6X_M4.h>
#include <ccm_imx6sx.h>
#include <clock.h>
#include <irq.h>
#include <vector.h>
#include <net/mac.h>
struct mac {
	ENET_Type *base;
	SemaphoreHandle_t mii;
};
struct mac fec_data = {
	.base = ENET1
};
void ENET1_Handler(void) {
	uint32_t eir = fec_data.base->EIR;
	fec_data.base->EIR = eir;
	BaseType_t higherPriorityTaskWoken = pdFALSE;
	if (eir & ENET_EIR_MII_MASK) {
		xSemaphoreGiveFromISR(fec_data.mii, &higherPriorityTaskWoken);
	}
	portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
void ENET2_Timer_Handler(void) {

}
#define ENET_MMFR_OP_WRTIE ENET_MMFR_OP(0x1)
#define ENET_MMFR_OP_READ ENET_MMFR_OP(0x2)
int32_t mac_mdioWrite(struct mac *mac, uint8_t id, uint8_t addr, uint16_t value) {
	BaseType_t ret;
	mac->base->MMFR = ENET_MMFR_ST(0x1) | ENET_MMFR_OP_WRTIE | ENET_MMFR_PA(id) | ENET_MMFR_TA(0x2) | ENET_MMFR_DATA(value) | ENET_MMFR_RA(addr);
	ret = xSemaphoreTake(mac->mii, 500 / portTICK_PERIOD_MS);
	if (ret != pdTRUE) {
		return -1;
	}
	return 0;
}
int32_t mac_mdioRead(struct mac *mac, uint8_t id, uint8_t addr, uint16_t *value) {
	BaseType_t ret;
	mac->base->MMFR = ENET_MMFR_ST(0x1) | ENET_MMFR_OP_READ | ENET_MMFR_PA(id) | ENET_MMFR_TA(0x2) | ENET_MMFR_RA(addr);
	ret = xSemaphoreTake(mac->mii, 500 / portTICK_PERIOD_MS);
	if (ret != pdTRUE) {
		return -1;
	}
	*value = (uint16_t) ENET_MMFR_DATA(mac->base->MMFR);
	return 0;	
}
void phytest_task() {
	int32_t ret;
	struct phy **phys;
	struct phy *phy;
	uint8_t len;
	uint32_t i;
	ret = phy_scan(&fec_data, &phys, &len);
	CONFIG_ASSERT(ret >= 0);
	printf("found %d phys: test phys\n", len);
	for (i = 0; i < len; i++) {
		phy = phys[i];
		CONFIG_ASSERT(phy != NULL);
		printf("id: %ld phyid: 0x%08lx\n", phy->id, phy->phyid);
		ret = phy_soft_reset(phy);
		CONFIG_ASSERT(ret >= 0);
		ret = phy_config_init(phy);
		CONFIG_ASSERT(ret >= 0);
		ret = phy_read_status(phy);
		CONFIG_ASSERT(ret >= 0);
		printf("Status: 0x%08lx\n", ret);
		ret = phy_config_aneg(phy);
		CONFIG_ASSERT(ret >= 0);
		printf("Wait for Aneg\n");
		do {
			ret = phy_aneg_done(phy);
		} while (ret == 0);
		printf("Wait for Aneg Compleate\n");
	}
	for(;;);
}
void phytest_init() {
	struct clock *clk = clock_init();
	uint32_t mii_speed = DIV_ROUND_UP(clock_getPeripherySpeed(clk), 5000000) - 1;
	uint32_t holdtime = DIV_ROUND_UP(clock_getPeripherySpeed(clk), 100000000) - 1;
	CONFIG_ASSERT(mii_speed <= 63);

	fec_data.mii = xSemaphoreCreateBinary();
	xSemaphoreGive(fec_data.mii);
	xSemaphoreTake(fec_data.mii, 0);

	CCM_ControlGate(CCM, ccmCcgrGateEnetClk, ccmClockNeededAll);
	fec_data.base->ECR |= ENET_ECR_RESET_MASK;
	fec_data.base->MSCR = ENET_MSCR_MII_SPEED(mii_speed) | ENET_MSCR_HOLDTIME(holdtime);
	/* Enable flow control and length check and RGII and 1G */
	fec_data.base->RCR |= 0x40000000 | 0x00000020 | BIT(6) | BIT(5);
	/* enable ENET store and forward mode */
	fec_data.base->TCR = BIT(8);
	fec_data.base->ECR = (ENET_ECR_ETHEREN_MASK | ENET_ECR_SPEED_MASK);	
	fec_data.base->EIMR |= ENET_EIMR_MII_MASK;
	irq_setPrio(NVIC_ENET1_HANDLER, 0xFF);
	irq_enable(NVIC_ENET1_HANDLER);
	BaseType_t ret = xTaskCreate(phytest_task, "phytest Task", 250, NULL, 2, NULL);
	CONFIG_ASSERT(ret == pdPASS);
}
