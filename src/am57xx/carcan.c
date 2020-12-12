#include <can.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>
#include <stdio.h>

void can_test() {
    struct can *can1 = can_init(CARCAN1_ID, 500000, NULL, false, false, NULL);
    printf("start can1 can_send\n");
    struct can_msg msg = {0, 2, 0, 8, {4,20, 69, 66, 21, 100, 13, 37}};
    can_send(can1, &msg, 1000);
    can_deinit(can1);


    struct can *can2 = can_init(CARCAN2_ID, 500000, NULL, false, false, NULL);
    printf("start can2 can_send\n");
    can_send(can2, &msg, 1000);
    can_deinit(can2);
}

