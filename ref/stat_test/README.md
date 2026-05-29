`stat_test` is an independent generator for OWF-output streams used in statistical tests.

What it does:
- Mimics plaintext behavior of `lynx_owf.c` for `lambda = 128, 192, 256, 384, 512`.
- Samples `L0/L1/L2/L3` matrices per-output using SHAKE and an AIMer-style
  lower/upper-triangular-product distribution.
- Uses variable `c0, c1` sampled uniformly via SHAKE.
- Samples `k` uniformly via SHAKE.
- Writes only OWF output `y` bytes to the output file.

Build:
```bash
cd stat_test
make
```

Run (default `bits = 2^33`):
```bash
./stat_test_gen --lambda 128 --out y_128.bin
./stat_test_gen --lambda 192 --out y_192.bin
./stat_test_gen --lambda 256 --out y_256.bin
./stat_test_gen --lambda 384 --out y_384.bin
./stat_test_gen --lambda 512 --out y_512.bin
```

Optional arguments:
```bash
./stat_test_gen --lambda 256 --out y.bin --bits 8589934592 --seed my-seed --threads 8
```

Notes:
- `bits` must be a multiple of 8.
- `threads` defaults to 1.
- For `2^33` bits, output size is `2^30` bytes.
