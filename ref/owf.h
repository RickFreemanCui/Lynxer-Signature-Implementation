/*
 *  SPDX-License-Identifier: MIT
 */

#ifndef OWF_H
#define OWF_H

#include "macros.h"

#include <stdint.h>

FAEST_BEGIN_C_DECL

void owf_128(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_192(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_256(const uint8_t* key, const uint8_t* input, uint8_t* output);

void owf_lynx_128(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_lynx_192(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_lynx_256(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_lynx_384(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_lynx_512(const uint8_t* key, const uint8_t* input, uint8_t* output);

// Witness extension for LYNX (placeholder until custom OWF is implemented)
struct faest_paramset_t;
void lynx_extend_witness(uint8_t* w, const uint8_t* key, const uint8_t* in,
                         const struct faest_paramset_t* params);

void owf_em_128(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_em_192(const uint8_t* key, const uint8_t* input, uint8_t* output);
void owf_em_256(const uint8_t* key, const uint8_t* input, uint8_t* output);

#define faest_128s_owf owf_128
#define faest_128f_owf owf_128
#define faest_192s_owf owf_192
#define faest_192f_owf owf_192
#define faest_256s_owf owf_256
#define faest_256f_owf owf_256

#define faest_lynx_128s_owf owf_lynx_128
#define faest_lynx_128f_owf owf_lynx_128
#define faest_lynx_192s_owf owf_lynx_192
#define faest_lynx_192f_owf owf_lynx_192
#define faest_lynx_256s_owf owf_lynx_256
#define faest_lynx_256f_owf owf_lynx_256
#define faest_lynx_384s_owf owf_lynx_384
#define faest_lynx_384f_owf owf_lynx_384
#define faest_lynx_512s_owf owf_lynx_512
#define faest_lynx_512f_owf owf_lynx_512

#define faest_em_128s_owf owf_em_128
#define faest_em_128f_owf owf_em_128
#define faest_em_192s_owf owf_em_192
#define faest_em_192f_owf owf_em_192
#define faest_em_256s_owf owf_em_256
#define faest_em_256f_owf owf_em_256

FAEST_END_C_DECL

#endif
