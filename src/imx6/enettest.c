#include <stdint.h>
#include <FreeRTOS.h>
#include <task.h>
#include <net/phy.h>
#include <net/mac.h>
#include <net/net.h>
#include <net/net_dummy.h>
#include <devs.h>
#include <string.h>
NET_DUMMY_ADDDEV(eth0);
NET_DUMMY_ADDDEV(eth1);
struct ethframe {
	uint32_t rxmacu;
	uint16_t rxmacl;
	uint32_t txmacu;
	uint16_t txmacl;
	uint16_t length;
	uint8_t payload[1024];

} PACKED;
struct ethframe eth;
void enettest_task(void *data) {
	int32_t ret;
	struct phy **phys;
	uint8_t len;
	struct mac **enets = data;
	struct net *nets[2] = {
		net_init(NET_DUMMY_ID(eth0)),
		net_init(NET_DUMMY_ID(eth1))
	};
	CONFIG_ASSERT(nets[0] != NULL);
	CONFIG_ASSERT(nets[1] != NULL);
	/* Scan for Phys */
	ret = phy_scan(enets[0], &phys, &len);
	CONFIG_ASSERT(ret >= 0);
	/* Check Scan */
	CONFIG_ASSERT(len == 2);
	CONFIG_ASSERT(phys[0] != NULL);
	CONFIG_ASSERT(phys[1] != NULL);
	/* Set Phys */
	ret = mac_setPhys(enets[0], phys, len);
	CONFIG_ASSERT(ret >= 0);
	/* Connect to Phy / Net and enable Mac Layer */
	ret = mac_connect(enets[0], phys[0], nets[0]);
	CONFIG_ASSERT(ret >= 0);
	/* Connect to Phy / Net and enable Mac Layer */
	ret = mac_connect(enets[1], phys[1], nets[1]);
	CONFIG_ASSERT(ret >= 0);
	ret = mac_enable(enets[0]);
	CONFIG_ASSERT(ret >= 0);
	ret = mac_enable(enets[1]);
	CONFIG_ASSERT(ret >= 0);
	vTaskDelay(5000 / portTICK_PERIOD_MS);
	{
		struct netbuff *buff;
		eth.rxmacu = cpu_to_be32(0xFFFFFFFF);
		eth.rxmacl = cpu_to_be16(0xFFFF);
		eth.txmacu = cpu_to_be32(0x12345678);
		eth.txmacl = cpu_to_be16(0x9ABC);
		eth.length = cpu_to_be16(1024);
		memset(eth.payload, 0x4242, sizeof(uint8_t) * 1024);
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
		{
			buff = net_allocNetbuff(nets[0], sizeof(struct ethframe));
			CONFIG_ASSERT(buff != NULL);
			memcpy(net_getPayload(nets[0], buff), &eth, sizeof(struct ethframe));
			ret = mac_send(enets[0], buff);
			CONFIG_ASSERT(ret == 0);
		}
	}
	for(;;);
}
struct mac *enets[2];
void enettest_init() {
	BaseType_t ret;
	enets[0] = mac_init(ENET1_ID);
	CONFIG_ASSERT(enets[0] != NULL);
	enets[1] = mac_init(ENET2_ID);
	CONFIG_ASSERT(enets[0] != NULL);
	ret = xTaskCreate(enettest_task, "enettest Task", 1000, enets, 2, NULL);
	CONFIG_ASSERT(ret == pdPASS);
}
