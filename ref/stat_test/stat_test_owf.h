/*
 *  SPDX-License-Identifier: MIT
 */

#ifndef STAT_TEST_OWF_H
#define STAT_TEST_OWF_H

#include <stddef.h>
#include <stdint.h>

int stat_owf_eval(unsigned int lambda, const uint8_t* k, const uint8_t* c0, const uint8_t* c1,
                  const uint8_t* mat_seed, size_t mat_seed_len, uint8_t* y);
size_t stat_owf_bytes(unsigned int lambda);

#endif
