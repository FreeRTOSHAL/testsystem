#include <can.h>
#include <devs.h>
#include <FreeRTOS.h>
#include <task.h>
#include <os.h>
#include <string.h>

void can_test() {
    struct can *can = CAN_INIT(CARCAN1_ID, 500000, NULL, false);
    CAN_DEINIT(can);
}

