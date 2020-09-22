/* SPDX-License-Identifier: MIT */
/*
 * Author: Andreas Werner <kernel@andy89.org>
 * Date: 2016
 */
#ifndef PPM_H_
#define PPM_H_
#include <gpio.h>
#include <stdint.h>
struct ppm *ppm_init(uint32_t slots, struct gpio_pin *pin);
int32_t ppm_start(struct ppm *ppm);
#endif
