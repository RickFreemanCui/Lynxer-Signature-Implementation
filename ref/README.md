# Lynx Reference Implementation

This implementation is based on the `faest-ref` repo (https://github.com/faest-sign/faest-ref).
We forked from the commit `808fbbad1e1ae32afac06d132368da83119be81e`.

We introduced the new `lynx` one-way function and the corresponding quicksilver proof system. We also used the Karatsuba algorithm for GF(2^n) multiplication.

If a linker error occurs on Ubuntu machines, try installing the following package might be helpful.

``` sh 
apt install libboost-program-options-dev
```

Building dependencies is outlined in the `Dependencies` section of `README.md` in the top directory.
After installing the dependencies, running 
```bash
bash ./bench.sh
```
will complete the compilation and benchmarking process.
The benchmark results will be recorded in `build_release_lu/bench_results.csv`.

The following is the original FAEST README.

```markdown

# FAEST - Reference implementation

## Dependencies

For building:
* `meson` version 0.57 or newer
* `ninja` (depending on the build system generator selected via `meson`)

For tests:
* `boost` (unit test framework)
* `NTL`

On Debian-based Linux distributions:
```sh
apt install meson ninja-build # for build dependencies
apt install libboost-test-dev libntl-dev # for test dependencies
```

Both `meson` and `ninja` are also available via PyPI:
```sh
pip install meson ninja
```

## Building

```sh
mkdir build
cd build
meson ..
ninja
ninja test
```
```