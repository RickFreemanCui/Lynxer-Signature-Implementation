/*
 *  SPDX-License-Identifier: MIT
 */

#include "stat_test_owf.h"

#include "../fields.h"
#include "../sha3/fips202-ref/fips202.h"

#include <string.h>

static inline int parity_u64(uint64_t x) {
  x ^= x >> 32;
  x ^= x >> 16;
  x ^= x >> 8;
  x ^= x >> 4;
  x ^= x >> 2;
  x ^= x >> 1;
  return (int)(x & 1);
}

#if defined(HAVE_ATTR_VECTOR_SIZE)
#define BF384_WORD(x, i) BF_VALUE((x).inner[(i) / 2], (i) % 2)
#else
#define BF384_WORD(x, i) BF_VALUE(x, i)
#endif

static inline bf128_t bf128_sq(bf128_t x) { return bf128_mul(x, x); }
static inline bf192_t bf192_sq(bf192_t x) { return bf192_mul(x, x); }
static inline bf256_t bf256_sq(bf256_t x) { return bf256_mul(x, x); }
static inline bf384_t bf384_sq(bf384_t x) { return bf384_mul(x, x); }
static inline bf512_t bf512_sq(bf512_t x) { return bf512_mul(x, x); }

#define STAT_MAX_LAMBDA 512
#define STAT_MAX_WORDS (STAT_MAX_LAMBDA / 64)

typedef struct {
  unsigned int lambda;
  keccak_state st;
} matrix_rng_t;

typedef struct {
  matrix_rng_t* rng;
  uint8_t buf[256];
  size_t bit_pos;
  size_t bit_len;
} bitstream_t;

static void matrix_rng_init(matrix_rng_t* rng, unsigned int lambda, const uint8_t* seed,
                            size_t seed_len) {
  static const uint8_t domain[] = "LYNX-STAT-MAT-v1";
  const uint8_t lambda_le[2] = {(uint8_t)(lambda & 0xff), (uint8_t)((lambda >> 8) & 0xff)};
  const uint8_t seed_len_le[2] = {(uint8_t)(seed_len & 0xff), (uint8_t)((seed_len >> 8) & 0xff)};
  rng->lambda = lambda;
  if (lambda == 128) {
    shake128_init(&rng->st);
    shake128_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake128_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake128_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake128_absorb(&rng->st, seed, seed_len);
    shake128_finalize(&rng->st);
  } else if (lambda == 192 || lambda == 256) {
    shake256_init(&rng->st);
    shake256_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake256_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake256_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake256_absorb(&rng->st, seed, seed_len);
    shake256_finalize(&rng->st);
  } else if (lambda == 384) {
    shake384_init(&rng->st);
    shake384_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake384_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake384_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake384_absorb(&rng->st, seed, seed_len);
    shake384_finalize(&rng->st);
  } else {
    shake512_init(&rng->st);
    shake512_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake512_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake512_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake512_absorb(&rng->st, seed, seed_len);
    shake512_finalize(&rng->st);
  }
}

static void matrix_rng_squeeze(matrix_rng_t* rng, uint8_t* out, size_t outlen) {
  if (rng->lambda == 128) {
    shake128_squeeze(out, outlen, &rng->st);
  } else if (rng->lambda == 192 || rng->lambda == 256) {
    shake256_squeeze(out, outlen, &rng->st);
  } else if (rng->lambda == 384) {
    shake384_squeeze(out, outlen, &rng->st);
  } else {
    shake512_squeeze(out, outlen, &rng->st);
  }
}

static inline void matrix_set_bit(uint64_t row[STAT_MAX_WORDS], unsigned int col) {
  row[col / 64] |= (UINT64_C(1) << (col % 64));
}

static inline int matrix_get_bit(const uint64_t row[STAT_MAX_WORDS], unsigned int col) {
  return (int)((row[col / 64] >> (col % 64)) & UINT64_C(1));
}

static inline void matrix_row_xor(uint64_t dst[STAT_MAX_WORDS], const uint64_t src[STAT_MAX_WORDS],
                                  unsigned int words) {
  for (unsigned int i = 0; i < words; ++i) {
    dst[i] ^= src[i];
  }
}

static void bitstream_init(bitstream_t* bs, matrix_rng_t* rng) {
  bs->rng = rng;
  bs->bit_pos = 0;
  bs->bit_len = 0;
}

static int bitstream_get_bit(bitstream_t* bs) {
  if (bs->bit_pos >= bs->bit_len) {
    matrix_rng_squeeze(bs->rng, bs->buf, sizeof(bs->buf));
    bs->bit_pos = 0;
    bs->bit_len = sizeof(bs->buf) * 8;
  }
  const size_t byte_pos = bs->bit_pos / 8;
  const unsigned int bit_pos = (unsigned int)(bs->bit_pos % 8);
  const int bit = (int)((bs->buf[byte_pos] >> bit_pos) & 1u);
  ++bs->bit_pos;
  return bit;
}

static void sample_lower_triangular(unsigned int lambda, unsigned int words, bitstream_t* bs,
                                    uint64_t L[STAT_MAX_LAMBDA][STAT_MAX_WORDS]) {
  memset(L, 0, sizeof(uint64_t) * STAT_MAX_LAMBDA * STAT_MAX_WORDS);
  for (unsigned int r = 0; r < lambda; ++r) {
    for (unsigned int c = 0; c < r; ++c) {
      if (bitstream_get_bit(bs)) {
        matrix_set_bit(L[r], c);
      }
    }
    matrix_set_bit(L[r], r);
  }
  (void)words;
}

static void sample_upper_triangular(unsigned int lambda, unsigned int words, bitstream_t* bs,
                                    uint64_t U[STAT_MAX_LAMBDA][STAT_MAX_WORDS]) {
  memset(U, 0, sizeof(uint64_t) * STAT_MAX_LAMBDA * STAT_MAX_WORDS);
  for (unsigned int r = 0; r < lambda; ++r) {
    matrix_set_bit(U[r], r);
    for (unsigned int c = r + 1; c < lambda; ++c) {
      if (bitstream_get_bit(bs)) {
        matrix_set_bit(U[r], c);
      }
    }
  }
  (void)words;
}

static void matrix_mul_gf2(unsigned int lambda, unsigned int words,
                           const uint64_t A[STAT_MAX_LAMBDA][STAT_MAX_WORDS],
                           const uint64_t B[STAT_MAX_LAMBDA][STAT_MAX_WORDS],
                           uint64_t C[STAT_MAX_LAMBDA][STAT_MAX_WORDS]) {
  memset(C, 0, sizeof(uint64_t) * STAT_MAX_LAMBDA * STAT_MAX_WORDS);
  for (unsigned int r = 0; r < lambda; ++r) {
    for (unsigned int c = 0; c < lambda; ++c) {
      if (matrix_get_bit(A[r], c)) {
        matrix_row_xor(C[r], B[c], words);
      }
    }
  }
}

static void sample_lu_product_matrices(unsigned int lambda, const uint8_t* mat_seed,
                                       size_t mat_seed_len,
                                       uint64_t L0[STAT_MAX_LAMBDA][STAT_MAX_WORDS],
                                       uint64_t L1[STAT_MAX_LAMBDA][STAT_MAX_WORDS],
                                       uint64_t L2[STAT_MAX_LAMBDA][STAT_MAX_WORDS],
                                       uint64_t L3[STAT_MAX_LAMBDA][STAT_MAX_WORDS]) {
  const unsigned int words = lambda / 64;
  matrix_rng_t rng;
  bitstream_t bs;
  uint64_t lower[STAT_MAX_LAMBDA][STAT_MAX_WORDS];
  uint64_t upper[STAT_MAX_LAMBDA][STAT_MAX_WORDS];
  uint64_t (*out[4])[STAT_MAX_WORDS] = {L0, L1, L2, L3};

  matrix_rng_init(&rng, lambda, mat_seed, mat_seed_len);
  bitstream_init(&bs, &rng);
  for (unsigned int i = 0; i < 4; ++i) {
    sample_lower_triangular(lambda, words, &bs, lower);
    sample_upper_triangular(lambda, words, &bs, upper);
    matrix_mul_gf2(lambda, words, upper, lower, out[i]);
  }
}

static bf128_t lynx_L128(const uint64_t M[STAT_MAX_LAMBDA][STAT_MAX_WORDS], bf128_t x) {
  bf128_t res = bf128_zero();
  for (unsigned int i = 0; i < 128; i++) {
    uint64_t dot = (M[i][0] & BF_VALUE(x, 0)) ^ (M[i][1] & BF_VALUE(x, 1));
    if (parity_u64(dot)) {
      BF_VALUE(res, i / 64) ^= UINT64_C(1) << (i % 64);
    }
  }
  return res;
}

static bf192_t lynx_L192(const uint64_t M[STAT_MAX_LAMBDA][STAT_MAX_WORDS], bf192_t x) {
  bf192_t res = bf192_zero();
  for (unsigned int i = 0; i < 192; i++) {
    uint64_t dot = (M[i][0] & BF_VALUE(x, 0)) ^ (M[i][1] & BF_VALUE(x, 1)) ^
                   (M[i][2] & BF_VALUE(x, 2));
    if (parity_u64(dot)) {
      BF_VALUE(res, i / 64) ^= UINT64_C(1) << (i % 64);
    }
  }
  return res;
}

static bf256_t lynx_L256(const uint64_t M[STAT_MAX_LAMBDA][STAT_MAX_WORDS], bf256_t x) {
  bf256_t res = bf256_zero();
  for (unsigned int i = 0; i < 256; i++) {
    uint64_t dot = (M[i][0] & BF_VALUE(x, 0)) ^ (M[i][1] & BF_VALUE(x, 1)) ^
                   (M[i][2] & BF_VALUE(x, 2)) ^ (M[i][3] & BF_VALUE(x, 3));
    if (parity_u64(dot)) {
      BF_VALUE(res, i / 64) ^= UINT64_C(1) << (i % 64);
    }
  }
  return res;
}

static bf384_t lynx_L384(const uint64_t M[STAT_MAX_LAMBDA][STAT_MAX_WORDS], bf384_t x) {
  bf384_t res = bf384_zero();
  for (unsigned int i = 0; i < 384; i++) {
    uint64_t dot = (M[i][0] & BF384_WORD(x, 0)) ^ (M[i][1] & BF384_WORD(x, 1)) ^
                   (M[i][2] & BF384_WORD(x, 2)) ^ (M[i][3] & BF384_WORD(x, 3)) ^
                   (M[i][4] & BF384_WORD(x, 4)) ^ (M[i][5] & BF384_WORD(x, 5));
    if (parity_u64(dot)) {
      BF384_WORD(res, i / 64) ^= UINT64_C(1) << (i % 64);
    }
  }
  return res;
}

static bf512_t lynx_L512(const uint64_t M[STAT_MAX_LAMBDA][STAT_MAX_WORDS], bf512_t x) {
  bf512_t res = bf512_zero();
  for (unsigned int i = 0; i < 512; i++) {
    uint64_t dot = 0;
    for (unsigned int j = 0; j < 8; ++j) {
      dot ^= (M[i][j] & BF_VALUE(x, j));
    }
    if (parity_u64(dot)) {
      BF_VALUE(res, i / 64) ^= UINT64_C(1) << (i % 64);
    }
  }
  return res;
}

static bf128_t bf128_inv_itoh(bf128_t x) {
  bf128_t t;
  bf128_t r1  = x;
  bf128_t r2  = bf128_mul(bf128_sq(r1), r1);
  t           = bf128_sq(bf128_sq(r2));
  bf128_t r4  = bf128_mul(t, r2);
  t = r4; for (int i = 0; i < 4; i++) t = bf128_sq(t);
  bf128_t r8  = bf128_mul(t, r4);
  t = r8; for (int i = 0; i < 8; i++) t = bf128_sq(t);
  bf128_t r16 = bf128_mul(t, r8);
  t = r16; for (int i = 0; i < 16; i++) t = bf128_sq(t);
  bf128_t r32 = bf128_mul(t, r16);
  t = r32; for (int i = 0; i < 32; i++) t = bf128_sq(t);
  bf128_t r64 = bf128_mul(t, r32);
  bf128_t r3  = bf128_mul(bf128_sq(r2), r1);
  t = r4; for (int i = 0; i < 3; i++) t = bf128_sq(t);
  bf128_t r7  = bf128_mul(t, r3);
  t = r8; for (int i = 0; i < 7; i++) t = bf128_sq(t);
  bf128_t r15 = bf128_mul(t, r7);
  t = r16; for (int i = 0; i < 15; i++) t = bf128_sq(t);
  bf128_t r31 = bf128_mul(t, r15);
  t = r32; for (int i = 0; i < 31; i++) t = bf128_sq(t);
  bf128_t r63 = bf128_mul(t, r31);
  t = r64; for (int i = 0; i < 63; i++) t = bf128_sq(t);
  bf128_t r127 = bf128_mul(t, r63);
  return bf128_sq(r127);
}

static bf192_t bf192_s1(bf192_t x) {
  bf192_t t;
  bf192_t r1  = x;
  bf192_t r2  = bf192_mul(bf192_sq(r1), r1);
  t           = bf192_sq(bf192_sq(r2));
  bf192_t r4  = bf192_mul(t, r2);
  t = r4; for (int i = 0; i < 4; i++) t = bf192_sq(t);
  bf192_t r8  = bf192_mul(t, r4);
  t = r8; for (int i = 0; i < 8; i++) t = bf192_sq(t);
  bf192_t r16 = bf192_mul(t, r8);
  t = r16; for (int i = 0; i < 16; i++) t = bf192_sq(t);
  bf192_t r32 = bf192_mul(t, r16);
  bf192_t r3  = bf192_mul(bf192_sq(r2), r1);
  t = r8; for (int i = 0; i < 3; i++) t = bf192_sq(t);
  bf192_t r11 = bf192_mul(t, r3);
  t = r16; for (int i = 0; i < 11; i++) t = bf192_sq(t);
  bf192_t r27 = bf192_mul(t, r11);
  t = r32; for (int i = 0; i < 27; i++) t = bf192_sq(t);
  return bf192_mul(t, r27);
}

static bf192_t bf192_s2(bf192_t x) {
  static const uint64_t e[3] = {
      UINT64_C(0xb6ddb6edb76dbb6d),
      UINT64_C(0x6dbb6ddb6edb76db),
      UINT64_C(0xdb76dbb6ddb6edb7),
  };
  bf192_t result = bf192_one();
  for (int i = 191; i >= 0; i--) {
    result = bf192_sq(result);
    if ((e[i / 64] >> (i % 64)) & 1) {
      result = bf192_mul(result, x);
    }
  }
  return result;
}

static bf256_t bf256_s1(bf256_t x) {
  bf256_t t;
  bf256_t r1  = x;
  bf256_t r2  = bf256_mul(bf256_sq(r1), r1);
  t           = bf256_sq(bf256_sq(r2));
  bf256_t r4  = bf256_mul(t, r2);
  t = r4; for (int i = 0; i < 4; i++) t = bf256_sq(t);
  bf256_t r8  = bf256_mul(t, r4);
  t = r8; for (int i = 0; i < 8; i++) t = bf256_sq(t);
  bf256_t r16 = bf256_mul(t, r8);
  t = r16; for (int i = 0; i < 16; i++) t = bf256_sq(t);
  bf256_t r32 = bf256_mul(t, r16);
  t           = bf256_sq(bf256_sq(r8));
  bf256_t r10 = bf256_mul(t, r2);
  t = r16; for (int i = 0; i < 10; i++) t = bf256_sq(t);
  bf256_t r26 = bf256_mul(t, r10);
  t = r32; for (int i = 0; i < 26; i++) t = bf256_sq(t);
  bf256_t r58 = bf256_mul(t, r26);
  t = r58; for (int i = 0; i < 58; i++) t = bf256_sq(t);
  return bf256_mul(t, x);
}

static bf256_t bf256_s2(bf256_t x) {
  static const uint64_t e[4] = {
      UINT64_C(0xdef7bdef7bdef7bd),
      UINT64_C(0xbdef7bdef7bdef7b),
      UINT64_C(0x7bdef7bdef7bdef7),
      UINT64_C(0xf7bdef7bdef7bdef),
  };
  bf256_t result = bf256_one();
  for (int i = 255; i >= 0; i--) {
    result = bf256_sq(result);
    if ((e[i / 64] >> (i % 64)) & 1) {
      result = bf256_mul(result, x);
    }
  }
  return result;
}

static bf384_t bf384_s1(bf384_t x) {
  bf384_t t;
  bf384_t r1  = x;
  bf384_t r2  = bf384_mul(bf384_sq(r1), r1);
  t           = bf384_sq(bf384_sq(r2));
  bf384_t r4  = bf384_mul(t, r2);
  t = r4; for (int i = 0; i < 4; i++) t = bf384_sq(t);
  bf384_t r8  = bf384_mul(t, r4);
  t = r8; for (int i = 0; i < 8; i++) t = bf384_sq(t);
  bf384_t r16 = bf384_mul(t, r8);
  bf384_t r6  = bf384_mul(bf384_sq(bf384_sq(r4)), r2);
  t = r8; for (int i = 0; i < 6; i++) t = bf384_sq(t);
  bf384_t r14 = bf384_mul(t, r6);
  t = r16; for (int i = 0; i < 14; i++) t = bf384_sq(t);
  bf384_t r30 = bf384_mul(t, r14);
  t = r30; for (int i = 0; i < 30; i++) t = bf384_sq(t);
  return bf384_mul(t, x);
}

static bf384_t bf384_s2(bf384_t x) {
  static const uint64_t e[6] = {
      UINT64_C(0x4a95ad5a94a95ad5),
      UINT64_C(0x95ad5a94a95ad5a9),
      UINT64_C(0xad5a94a95ad5a94a),
      UINT64_C(0x5a94a95ad5a94a95),
      UINT64_C(0x94a95ad5a94a95ad),
      UINT64_C(0xa95ad5a94a95ad5a),
  };
  bf384_t result = bf384_one();
  for (int i = 383; i >= 0; i--) {
    result = bf384_sq(result);
    if ((e[i / 64] >> (i % 64)) & 1) {
      result = bf384_mul(result, x);
    }
  }
  return result;
}

static bf512_t bf512_s1(bf512_t x) {
  bf512_t t;
  bf512_t r1  = x;
  bf512_t r2  = bf512_mul(bf512_sq(r1), r1);
  t           = bf512_sq(bf512_sq(r2));
  bf512_t r4  = bf512_mul(t, r2);
  t = r4; for (int i = 0; i < 4; i++) t = bf512_sq(t);
  bf512_t r8  = bf512_mul(t, r4);
  t = r8; for (int i = 0; i < 8; i++) t = bf512_sq(t);
  bf512_t r16 = bf512_mul(t, r8);
  bf512_t r6  = bf512_mul(bf512_sq(bf512_sq(r4)), r2);
  t = r8; for (int i = 0; i < 6; i++) t = bf512_sq(t);
  bf512_t r14 = bf512_mul(t, r6);
  t = r16; for (int i = 0; i < 14; i++) t = bf512_sq(t);
  bf512_t r30 = bf512_mul(t, r14);
  t = r30; for (int i = 0; i < 30; i++) t = bf512_sq(t);
  return bf512_mul(t, x);
}

static bf512_t bf512_s2(bf512_t x) {
  static const uint64_t e[8] = {
      UINT64_C(0xc9898d9d9c9898d9),
      UINT64_C(0x898d9d9c9898d9d9),
      UINT64_C(0x8d9d9c9898d9d9c9),
      UINT64_C(0x9d9c9898d9d9c989),
      UINT64_C(0x9c9898d9d9c9898d),
      UINT64_C(0x9898d9d9c9898d9d),
      UINT64_C(0x98d9d9c9898d9d9c),
      UINT64_C(0xd9d9c9898d9d9c98),
  };
  bf512_t result = bf512_one();
  for (int i = 511; i >= 0; i--) {
    result = bf512_sq(result);
    if ((e[i / 64] >> (i % 64)) & UINT64_C(1)) {
      result = bf512_mul(result, x);
    }
  }
  return result;
}

static bf512_t bf512_inv_allones_exp(bf512_t x) {
  bf512_t result = bf512_one();
  for (int i = 511; i >= 0; --i) {
    result = bf512_sq(result);
    if (i != 0) {
      result = bf512_mul(result, x);
    }
  }
  return result;
}

size_t stat_owf_bytes(unsigned int lambda) {
  switch (lambda) {
  case 128: return 16;
  case 192: return 24;
  case 256: return 32;
  case 384: return 48;
  case 512: return 64;
  default: return 0;
  }
}

int stat_owf_eval(unsigned int lambda, const uint8_t* k_bytes, const uint8_t* c0_bytes,
                  const uint8_t* c1_bytes, const uint8_t* mat_seed, size_t mat_seed_len,
                  uint8_t* y_bytes) {
  uint64_t L0[STAT_MAX_LAMBDA][STAT_MAX_WORDS];
  uint64_t L1[STAT_MAX_LAMBDA][STAT_MAX_WORDS];
  uint64_t L2[STAT_MAX_LAMBDA][STAT_MAX_WORDS];
  uint64_t L3[STAT_MAX_LAMBDA][STAT_MAX_WORDS];

  if (!k_bytes || !c0_bytes || !c1_bytes || !mat_seed || mat_seed_len == 0 || !y_bytes) {
    return -1;
  }
  sample_lu_product_matrices(lambda, mat_seed, mat_seed_len, L0, L1, L2, L3);

  switch (lambda) {
  case 128: {
    const bf128_t k  = bf128_load(k_bytes);
    const bf128_t c0 = bf128_load(c0_bytes);
    const bf128_t c1 = bf128_load(c1_bytes);
    const bf128_t v1 = bf128_inv_itoh(bf128_add(k, c0));
    const bf128_t v2 = bf128_inv_itoh(bf128_add(lynx_L128(L0, k), c1));
    const bf128_t y =
        bf128_add(bf128_mul(lynx_L128(L1, v1), lynx_L128(L2, v2)),
                  bf128_add(lynx_L128(L3, k),
                            bf128_add(lynx_L128(L3, v1), lynx_L128(L3, v2))));
    bf128_store(y_bytes, y);
    return 0;
  }
  case 192: {
    const bf192_t k  = bf192_load(k_bytes);
    const bf192_t c0 = bf192_load(c0_bytes);
    const bf192_t c1 = bf192_load(c1_bytes);
    const bf192_t v1 = bf192_s1(bf192_add(k, c0));
    const bf192_t v2 = bf192_s2(bf192_add(lynx_L192(L0, k), c1));
    const bf192_t y =
        bf192_add(bf192_mul(lynx_L192(L1, v1), lynx_L192(L2, v2)),
                  bf192_add(lynx_L192(L3, k),
                            bf192_add(lynx_L192(L3, v1), lynx_L192(L3, v2))));
    bf192_store(y_bytes, y);
    return 0;
  }
  case 256: {
    const bf256_t k  = bf256_load(k_bytes);
    const bf256_t c0 = bf256_load(c0_bytes);
    const bf256_t c1 = bf256_load(c1_bytes);
    const bf256_t v1 = bf256_s1(bf256_add(k, c0));
    const bf256_t v2 = bf256_s2(bf256_add(lynx_L256(L0, k), c1));
    const bf256_t y =
        bf256_add(bf256_mul(lynx_L256(L1, v1), lynx_L256(L2, v2)),
                  bf256_add(lynx_L256(L3, k),
                            bf256_add(lynx_L256(L3, v1), lynx_L256(L3, v2))));
    bf256_store(y_bytes, y);
    return 0;
  }
  case 384: {
    const bf384_t k  = bf384_load(k_bytes);
    const bf384_t c0 = bf384_load(c0_bytes);
    const bf384_t c1 = bf384_load(c1_bytes);
    const bf384_t v1 = bf384_s1(bf384_add(k, c0));
    const bf384_t v2 = bf384_s2(bf384_add(lynx_L384(L0, k), c1));
    const bf384_t y =
        bf384_add(bf384_mul(lynx_L384(L1, v1), lynx_L384(L2, v2)),
                  bf384_add(lynx_L384(L3, k),
                            bf384_add(lynx_L384(L3, v1), lynx_L384(L3, v2))));
    bf384_store(y_bytes, y);
    return 0;
  }
  case 512: {
    const bf512_t k  = bf512_load(k_bytes);
    const bf512_t c0 = bf512_load(c0_bytes);
    const bf512_t c1 = bf512_load(c1_bytes);
    const bf512_t v1 = bf512_s1(bf512_add(k, c0));
    const bf512_t v2 = bf512_s2(bf512_add(lynx_L512(L0, k), c1));
    const bf512_t invk = bf512_inv_allones_exp(k);
    const bf512_t y = bf512_add(
        bf512_mul(invk, bf512_mul(lynx_L512(L1, v1), lynx_L512(L2, v2))),
        bf512_add(lynx_L512(L3, k),
                  bf512_add(lynx_L512(L3, v1), lynx_L512(L3, v2))));
    bf512_store(y_bytes, y);
    return 0;
  }
  default:
    return -1;
  }
}
