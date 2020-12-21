#include <can.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>
#include <stdio.h>

void can_test() {
    struct can *can1;
    struct can_msg msg_send = {0, 2, 0, 8, {4,20, 69, 66, 21, 100, 13, 37}};
    struct can_msg msg_recv;
    struct can_filter filter = {2, 0};
    int filterID;
    can1 = can_init(DCAN1_ID, 500000, NULL, false, false, NULL);
    printf("start can1 can_send\n");
    filterID = can_registerFilter(can1, &filter);
    can_send(can1, &msg_send, 10000);
    can_recv(can1,filterID, &msg_recv, 10000);
    can_deregisterFilter(can1, filterID);
    can_deinit(can1);

    /*
    struct can *can2 = can_init(DCAN2_ID, 500000, NULL, false, false, NULL);
    printf("start can2 can_send\n");
    can_send(can2, &msg, 1000);
    can_deinit(can2);
    */
}

