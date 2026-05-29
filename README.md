# Lynxer Signature Scheme Repository

This repository contains two implementations of the Lynxer signature scheme:

- `ref/`: reference implementation, adapted from the FAEST reference code (https://github.com/faest-sign/faest-ref)
- `opt/`: optimized implementation, adapted from the FAEST optimized code (https://github.com/faest-sign/faest-arch-opt)

Each subdirectory remains self-contained and keeps its own build configuration, tests, and implementation-specific documentation.

## Benchmarking

- `ref/`: run `bash ./bench.sh`
- `opt/`: run `bash ./scripts/benchmark-local.sh`
