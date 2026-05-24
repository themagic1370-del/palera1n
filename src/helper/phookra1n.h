#ifndef PALERA1N_CHECKRA1N_HELPER_H
#define PALERA1N_CHECKRA1N_HELPER_H

#include <stdint.h>
#include <stdio.h>

#define dprintf(...) printf(__VA_ARGS__)

extern uint8_t t7000_bin[];
extern uint32_t t7000_bin_len;

extern uint8_t t8011_bin[];
extern uint32_t t8011_bin_len;

extern uint8_t t8012_bin[];
extern uint32_t t8012_bin_len;

void setup_hooks_arm64(void *stream);

#endif
