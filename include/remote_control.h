#ifndef REMOTE_CONTROL_H_
#define REMOTE_CONTROL_H_
#include <stdint.h>
#define RC_MAX_CHANNELS 8
#define RC_MIN 480 
#define RC_MAX 2200
#include <flextimer.h>
struct rc;
struct rc *rc_init(struct timer *ftm);
int32_t rc_setup(struct rc *rc, uint32_t channel);
uint32_t rc_get(struct rc *rc, uint32_t channel);
#endif
