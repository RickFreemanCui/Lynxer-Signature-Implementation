/*
 *  SPDX-License-Identifier: MIT
 */

#include "stat_test_owf.h"

#include "../sha3/fips202-ref/fips202.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_BITS (UINT64_C(1) << 33)
#define DEFAULT_SEED "lynx-stat-test-default-seed"
#define DEFAULT_THREADS 1
#define GEN_BUFFER_BYTES (1u << 16)

typedef struct {
  unsigned int lambda;
  keccak_state st;
} shake_rng_t;

typedef struct {
  atomic_ullong written;
  atomic_int done;
  uint64_t total;
  struct timespec t0;
} progress_t;

typedef struct {
  unsigned int lambda;
  const char* seed_str;
  unsigned int thread_id;
  int fd;
  uint64_t start;
  uint64_t end;
  progress_t* progress;
  int status;
} worker_ctx_t;

typedef struct {
  progress_t* progress;
} progress_thread_ctx_t;

static int parse_u64(const char* s, uint64_t* out) {
  char* end = NULL;
  unsigned long long v = strtoull(s, &end, 10);
  if (!s[0] || (end && *end != '\0')) {
    return -1;
  }
  *out = (uint64_t)v;
  return 0;
}

static int parse_u32(const char* s, unsigned int* out) {
  uint64_t tmp = 0;
  if (parse_u64(s, &tmp) != 0 || tmp == 0 || tmp > 1024) {
    return -1;
  }
  *out = (unsigned int)tmp;
  return 0;
}

static int parse_lambda(const char* s, unsigned int* lambda) {
  if (strcmp(s, "128") == 0 || strcmp(s, "192") == 0 || strcmp(s, "256") == 0 ||
      strcmp(s, "384") == 0 || strcmp(s, "512") == 0) {
    *lambda = (unsigned int)atoi(s);
    return 0;
  }
  return -1;
}

static void rng_init(shake_rng_t* rng, unsigned int lambda, const uint8_t* seed, size_t seed_len,
                     uint64_t stream_id) {
  static const uint8_t domain[] = "LYNX-STAT-TEST-v1";
  const uint8_t lambda_le[2] = {(uint8_t)(lambda & 0xff), (uint8_t)((lambda >> 8) & 0xff)};
  const uint8_t seed_len_le[2] = {(uint8_t)(seed_len & 0xff), (uint8_t)((seed_len >> 8) & 0xff)};
  uint8_t stream_le[8];
  for (unsigned int i = 0; i < 8; ++i) {
    stream_le[i] = (uint8_t)((stream_id >> (8 * i)) & 0xff);
  }
  rng->lambda = lambda;

  if (lambda == 128) {
    shake128_init(&rng->st);
    shake128_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake128_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake128_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake128_absorb(&rng->st, seed, seed_len);
    shake128_absorb(&rng->st, stream_le, sizeof(stream_le));
    shake128_finalize(&rng->st);
  } else if (lambda == 192 || lambda == 256) {
    shake256_init(&rng->st);
    shake256_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake256_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake256_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake256_absorb(&rng->st, seed, seed_len);
    shake256_absorb(&rng->st, stream_le, sizeof(stream_le));
    shake256_finalize(&rng->st);
  } else if (lambda == 384) {
    shake384_init(&rng->st);
    shake384_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake384_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake384_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake384_absorb(&rng->st, seed, seed_len);
    shake384_absorb(&rng->st, stream_le, sizeof(stream_le));
    shake384_finalize(&rng->st);
  } else {
    shake512_init(&rng->st);
    shake512_absorb(&rng->st, domain, sizeof(domain) - 1);
    shake512_absorb(&rng->st, lambda_le, sizeof(lambda_le));
    shake512_absorb(&rng->st, seed_len_le, sizeof(seed_len_le));
    shake512_absorb(&rng->st, seed, seed_len);
    shake512_absorb(&rng->st, stream_le, sizeof(stream_le));
    shake512_finalize(&rng->st);
  }
}

static void rng_squeeze(shake_rng_t* rng, uint8_t* out, size_t outlen) {
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

static int pwrite_all(int fd, const uint8_t* buf, size_t len, uint64_t off) {
  size_t done = 0;
  while (done < len) {
    const ssize_t wr = pwrite(fd, buf + done, len - done, (off_t)(off + done));
    if (wr < 0) {
      if (errno == EINTR) {
        continue;
      }
      return -1;
    }
    done += (size_t)wr;
  }
  return 0;
}

static double sec_since(const struct timespec* t0) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  const double s = (double)(now.tv_sec - t0->tv_sec);
  const double ns = (double)(now.tv_nsec - t0->tv_nsec) / 1e9;
  return s + ns;
}

static void* progress_main(void* arg) {
  progress_thread_ctx_t* pctx = (progress_thread_ctx_t*)arg;
  progress_t* p = pctx->progress;
  while (!atomic_load_explicit(&p->done, memory_order_relaxed)) {
    sleep(2);
    const uint64_t w = atomic_load_explicit(&p->written, memory_order_relaxed);
    const double pct = p->total ? (100.0 * (double)w / (double)p->total) : 100.0;
    const double sec = sec_since(&p->t0);
    const double mib = (double)w / (1024.0 * 1024.0);
    const double mibps = sec > 0.0 ? mib / sec : 0.0;
    fprintf(stderr, "[progress] %.2f%% (%" PRIu64 "/%" PRIu64 " bytes, %.2f MiB/s)\n", pct, w,
            p->total, mibps);
  }

  {
    const uint64_t w = atomic_load_explicit(&p->written, memory_order_relaxed);
    const double sec = sec_since(&p->t0);
    const double mib = (double)w / (1024.0 * 1024.0);
    const double mibps = sec > 0.0 ? mib / sec : 0.0;
    fprintf(stderr, "[progress] 100.00%% (%" PRIu64 "/%" PRIu64 " bytes, %.2f MiB/s)\n", w,
            p->total, mibps);
  }
  return NULL;
}

static void* worker_main(void* arg) {
  worker_ctx_t* w = (worker_ctx_t*)arg;
  const size_t elem_bytes = stat_owf_bytes(w->lambda);
  if (elem_bytes == 0 || w->end < w->start) {
    w->status = -1;
    return NULL;
  }

  shake_rng_t rng;
  rng_init(&rng, w->lambda, (const uint8_t*)w->seed_str, strlen(w->seed_str), w->thread_id);

  uint8_t k[64] = {0};
  uint8_t c0[64] = {0};
  uint8_t c1[64] = {0};
  uint8_t mat_seed[64] = {0};
  uint8_t y[64] = {0};
  uint8_t* outbuf = (uint8_t*)malloc(GEN_BUFFER_BYTES);
  if (!outbuf) {
    w->status = -1;
    return NULL;
  }

  uint64_t pos = w->start;
  while (pos < w->end) {
    size_t chunk = (size_t)((w->end - pos) > GEN_BUFFER_BYTES ? GEN_BUFFER_BYTES : (w->end - pos));
    size_t filled = 0;
    while (filled < chunk) {
      rng_squeeze(&rng, k, elem_bytes);
      rng_squeeze(&rng, c0, elem_bytes);
      rng_squeeze(&rng, c1, elem_bytes);
      rng_squeeze(&rng, mat_seed, elem_bytes);
      if (stat_owf_eval(w->lambda, k, c0, c1, mat_seed, elem_bytes, y) != 0) {
        free(outbuf);
        w->status = -1;
        return NULL;
      }
      size_t take = elem_bytes;
      if (take > (chunk - filled)) {
        take = chunk - filled;
      }
      memcpy(outbuf + filled, y, take);
      filled += take;
    }
    if (pwrite_all(w->fd, outbuf, filled, pos) != 0) {
      free(outbuf);
      w->status = -1;
      return NULL;
    }
    atomic_fetch_add_explicit(&w->progress->written, (unsigned long long)filled,
                              memory_order_relaxed);
    pos += filled;
  }

  free(outbuf);
  w->status = 0;
  return NULL;
}

static void usage(const char* prog) {
  fprintf(stderr,
          "Usage: %s --lambda <128|192|256|384|512> --out <file> [--bits <n>] [--seed <str>] [--threads <n>]\n",
          prog);
}

int main(int argc, char** argv) {
  unsigned int lambda = 0;
  const char* out_path = NULL;
  const char* seed_str = DEFAULT_SEED;
  uint64_t total_bits = DEFAULT_BITS;
  unsigned int threads = DEFAULT_THREADS;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--lambda") == 0 && i + 1 < argc) {
      if (parse_lambda(argv[++i], &lambda) != 0) {
        usage(argv[0]);
        return 1;
      }
    } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
      out_path = argv[++i];
    } else if (strcmp(argv[i], "--bits") == 0 && i + 1 < argc) {
      if (parse_u64(argv[++i], &total_bits) != 0) {
        usage(argv[0]);
        return 1;
      }
    } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
      seed_str = argv[++i];
    } else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc) {
      if (parse_u32(argv[++i], &threads) != 0) {
        usage(argv[0]);
        return 1;
      }
    } else {
      usage(argv[0]);
      return 1;
    }
  }

  if (!out_path || lambda == 0 || (total_bits & 7u)) {
    usage(argv[0]);
    return 1;
  }

  const size_t elem_bytes = stat_owf_bytes(lambda);
  if (elem_bytes == 0) {
    fprintf(stderr, "Unsupported lambda\n");
    return 1;
  }

  const uint64_t total_bytes = total_bits / 8;
  const int fd = open(out_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  if (ftruncate(fd, (off_t)total_bytes) != 0) {
    perror("ftruncate");
    close(fd);
    return 1;
  }

  if (threads > total_bytes) {
    threads = (unsigned int)total_bytes;
    if (threads == 0) {
      threads = 1;
    }
  }

  pthread_t* tids = (pthread_t*)calloc(threads, sizeof(pthread_t));
  worker_ctx_t* ctx = (worker_ctx_t*)calloc(threads, sizeof(worker_ctx_t));
  pthread_t progress_tid;
  progress_thread_ctx_t progress_ctx;
  progress_t progress;
  if (!tids || !ctx) {
    fprintf(stderr, "allocation failed\n");
    free(tids);
    free(ctx);
    close(fd);
    return 1;
  }

  const uint64_t chunk = total_bytes / threads;
  const uint64_t rem = total_bytes % threads;
  atomic_init(&progress.written, 0);
  atomic_init(&progress.done, 0);
  progress.total = total_bytes;
  clock_gettime(CLOCK_MONOTONIC, &progress.t0);
  progress_ctx.progress = &progress;
  if (pthread_create(&progress_tid, NULL, progress_main, &progress_ctx) != 0) {
    fprintf(stderr, "pthread_create progress failed\n");
    close(fd);
    free(tids);
    free(ctx);
    return 1;
  }

  uint64_t start = 0;
  for (unsigned int t = 0; t < threads; ++t) {
    const uint64_t len = chunk + (t < rem ? 1 : 0);
    ctx[t].lambda = lambda;
    ctx[t].seed_str = seed_str;
    ctx[t].thread_id = t;
    ctx[t].fd = fd;
    ctx[t].start = start;
    ctx[t].end = start + len;
    ctx[t].progress = &progress;
    ctx[t].status = -1;
    start += len;
    if (pthread_create(&tids[t], NULL, worker_main, &ctx[t]) != 0) {
      fprintf(stderr, "pthread_create failed\n");
      close(fd);
      free(tids);
      free(ctx);
      return 1;
    }
  }

  int ok = 1;
  for (unsigned int t = 0; t < threads; ++t) {
    pthread_join(tids[t], NULL);
    if (ctx[t].status != 0) {
      ok = 0;
    }
  }
  atomic_store_explicit(&progress.done, 1, memory_order_relaxed);
  pthread_join(progress_tid, NULL);

  close(fd);
  free(tids);
  free(ctx);

  if (!ok) {
    fprintf(stderr, "generation failed in at least one thread\n");
    return 1;
  }

  const uint64_t samples = (total_bytes + elem_bytes - 1) / elem_bytes;
  fprintf(stdout,
          "done lambda=%u bits=%" PRIu64 " bytes=%" PRIu64 " samples=%" PRIu64
          " threads=%u out=%s\n",
          lambda, total_bits, total_bytes, samples, threads, out_path);
  return 0;
}
