# Lynxer Signature Scheme Repository

This repository contains two implementations of the Lynxer signature scheme:

- `ref/`: reference implementation, adapted from the FAEST reference code (https://github.com/faest-sign/faest-ref)
- `opt/`: optimized implementation, adapted from the FAEST optimized code (https://github.com/faest-sign/faest-arch-opt)

Each subdirectory remains self-contained and keeps its own build configuration, tests, and implementation-specific documentation.

## Dependencies

Both `ref/` and `opt/` use Meson/Ninja, but they have different toolchain requirements.

### Reference Implementation

Minimum requirements:

- C11 and C++17 compiler support
- Meson >= 0.59 and Ninja
- Python 3
- Boost >= 1.81 for tests and benchmarks (`unit_test_framework`, `program_options`)
- OpenSSL development headers for the optional OpenSSL backend and KAT generators
- NTL for the optional field tests
- `jq` and `bc` for `ref/bench.sh`

On Debian/Ubuntu:

```sh
sudo apt update
sudo apt install build-essential meson ninja-build python3 pkg-config \
    libssl-dev libboost-dev libboost-test-dev libboost-program-options-dev \
    libntl-dev jq bc
```

### Optimized Implementation

Minimum requirements:

- x86-64 CPU with AVX2, AES-NI, PCLMULQDQ, and BMI1
- GCC >= 14.2.0 or Clang >= 19.1.7
- Meson >= 1.7.0 and Ninja >= 1.12.1
- Python 3 and `pkg-config`
- Network access during Meson setup to fetch XKCP and Catch2, unless they are already present under `opt/subprojects/`

On Debian/Ubuntu:

```sh
sudo apt update
sudo apt install gcc-14 g++-14 meson ninja-build python3 pkg-config
```

If the distribution packages are older than the versions above, install newer Meson/Ninja via PyPI
or use a newer Debian/Ubuntu release. If the default GCC is already >= 14.2.0, `build-essential`
can be used instead of explicitly installing `gcc-14` and `g++-14`.

The optimized build downloads XKCP at revision `64404beeeb261b08a1076fe2f076e4e28dd9b040` and
Catch2 3.7.1 through Meson wraps.

### Verified Environment

The implementations were tested on an x86-64 machine:

- OS: Ubuntu 26.04 LTS, Linux 7.0.0-15-generic, x86_64
- Meson 1.10.1, Ninja 1.13.2, GCC/G++ 15.2.0
- Python 3.14.4, `pkg-config` 2.5.1
- OpenSSL 3.5.5, Boost 1.90, NTL 11.5.1
- `jq` 1.8.1, `bc` 1.07.1

## Benchmarking

- In `ref/`: run `bash ./bench.sh`
- In `opt/`: run `bash ./scripts/benchmark-local.sh`
