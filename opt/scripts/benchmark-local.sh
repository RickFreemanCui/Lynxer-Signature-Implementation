#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-build_bench}"
OUTPUT_DIR="${ROOT_DIR}/reports/benchmarks"
OUTPUT_FILE="${1:-${OUTPUT_DIR}/bench_result_$(date -Idate).csv}"
OUTPUT_STEM="${OUTPUT_FILE%.csv}"

MESON_OPTS=(
    --buildtype=release
    -Db_lto=false
    -Doptimization=3
)

VARIANTS=(
    "lynx::lynx_128_s"
    "lynx::lynx_128_f"
    "lynx::lynx_192_s"
    "lynx::lynx_192_f"
    "lynx::lynx_256_s"
    "lynx::lynx_256_f"
    "lynx::lynx_384_s"
    "lynx::lynx_384_f"
    "lynx::lynx_512_s"
    "lynx::lynx_512_f"
)

cd "${ROOT_DIR}"

mkdir -p "${OUTPUT_DIR}"

if [[ ! -d "${BUILD_DIR}" ]]; then
    meson setup "${BUILD_DIR}" "${MESON_OPTS[@]}"
else
    meson configure "${BUILD_DIR}" "${MESON_OPTS[@]}"
fi

meson compile -C "${BUILD_DIR}" test/bench

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

{
    echo "variant,scheme,variant_kind,pk_bytes,sk_bytes,sig_bytes,keygen,sign,verify,secpar,tau,delta_bits,open_threshold"
} > "${OUTPUT_FILE}"

for variant in "${VARIANTS[@]}"; do
    log_file="${TMP_DIR}/$(echo "${variant}" | tr ':/' '__').log"
    echo "=== ${variant} ==="
    "./${BUILD_DIR}/test/bench" "bench variants - ${variant}" --abort --durations yes | tee "${log_file}"

    pk_bytes="$(sed -n 's/.*"pk_size": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"
    sk_bytes="$(sed -n 's/.*"sk_size": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"
    sig_bytes="$(sed -n 's/.*"sig_size": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"
    secpar="$(sed -n 's/.*"secpar": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"
    tau="$(sed -n 's/.*"tau": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"
    delta_bits="$(sed -n 's/.*"delta_bits": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"
    open_threshold="$(sed -n 's/.*"open_threshold": \([0-9]\+\).*/\1/p' "${log_file}" | tail -n1)"

    if [[ "${variant}" == v2::faest_em_* ]]; then
        scheme="faest-em"
    elif [[ "${variant}" == v2::faest_* ]]; then
        scheme="faest"
    elif [[ "${variant}" == lynx::lynx_* ]]; then
        scheme="lynx"
    else
        scheme="unknown"
    fi
    variant_kind="${variant##*_}"

    keygen="$(awk '/^[[:space:]]*keygen[[:space:]]+/ {getline; print $1, $2; exit}' "${log_file}")"
    sign="$(awk '/^[[:space:]]*sign[[:space:]]+/ {getline; print $1, $2; exit}' "${log_file}")"
    verify="$(awk '/^[[:space:]]*verify[[:space:]]+/ {getline; print $1, $2; exit}' "${log_file}")"

    printf '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n' \
        "${variant}" "${scheme}" "${variant_kind}" "${pk_bytes}" "${sk_bytes}" "${sig_bytes}" \
        "${keygen}" "${sign}" "${verify}" \
        "${secpar}" "${tau}" "${delta_bits}" "${open_threshold}" >> "${OUTPUT_FILE}"
done

echo
echo "Benchmark CSV written to ${OUTPUT_FILE}"

python3 "${ROOT_DIR}/scripts/plot-bench-results.py" "${OUTPUT_FILE}" "${OUTPUT_STEM}"
