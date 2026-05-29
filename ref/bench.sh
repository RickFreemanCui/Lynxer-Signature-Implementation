#!/bin/bash

set -euo pipefail
# set -euxo pipefail

FAEST_128S="faest_128s"
FAEST_128F="faest_128f"
FAEST_192S="faest_192s"
FAEST_192F="faest_192f"
FAEST_256S="faest_256s"
FAEST_256F="faest_256f"
FAEST_EM_128S="faest_em_128s"
FAEST_EM_128F="faest_em_128f"
FAEST_EM_192S="faest_em_192s"
FAEST_EM_192F="faest_em_192f"
FAEST_EM_256S="faest_em_256s"
FAEST_EM_256F="faest_em_256f"
FAEST_LYNX_128S="faest_lynx_128s"
FAEST_LYNX_128F="faest_lynx_128f"
FAEST_LYNX_192S="faest_lynx_192s"
FAEST_LYNX_192F="faest_lynx_192f"
FAEST_LYNX_256S="faest_lynx_256s"
FAEST_LYNX_256F="faest_lynx_256f"
FAEST_LYNX_384S="faest_lynx_384s"
FAEST_LYNX_384F="faest_lynx_384f"
FAEST_LYNX_512S="faest_lynx_512s"
FAEST_LYNX_512F="faest_lynx_512f"

ALL="FAEST_128S FAEST_128F FAEST_192S FAEST_192F FAEST_256S FAEST_256F FAEST_EM_128S FAEST_EM_128F FAEST_EM_192S FAEST_EM_192F FAEST_EM_256S FAEST_EM_256F FAEST_LYNX_128S FAEST_LYNX_128F FAEST_LYNX_192S FAEST_LYNX_192F FAEST_LYNX_256S FAEST_LYNX_256F FAEST_LYNX_384S FAEST_LYNX_384F FAEST_LYNX_512S FAEST_LYNX_512F"
BENCH_VARIANTS="${BENCH_VARIANTS:-}"
if [ -n "${BENCH_VARIANTS}" ]; then
    ALL="${BENCH_VARIANTS}"
fi

BUILD_DIR="build_release"
LYNX_MATRIX_MODE="${LYNX_MATRIX_MODE:-LU}"
RUN_TESTS="${RUN_TESTS:-false}"
if [ "${LYNX_MATRIX_MODE}" != "LU" ] && [ "${LYNX_MATRIX_MODE}" != "FIXED" ]; then
    echo "Invalid LYNX_MATRIX_MODE='${LYNX_MATRIX_MODE}', expected 'LU' or 'FIXED'" >&2
    exit 1
fi
if [ "${RUN_TESTS}" != "true" ] && [ "${RUN_TESTS}" != "false" ]; then
    echo "Invalid RUN_TESTS='${RUN_TESTS}', expected 'true' or 'false'" >&2
    exit 1
fi
BUILD_DIR="build_release_`echo "${LYNX_MATRIX_MODE}" | tr '[:upper:]' '[:lower:]'`"
RESULT_JSONL="${BUILD_DIR}/bench_results.jsonl"
RESULT_CSV="${BUILD_DIR}/bench_results.csv"
BENCH_ITER="${BENCH_ITER:-100}"

# Prefer modern Homebrew Boost on Apple Silicon when present.
if [ -d "/opt/homebrew/include/boost" ] && [ -d "/opt/homebrew/lib" ]; then
    export BOOST_ROOT="/opt/homebrew"
    export BOOST_INCLUDEDIR="/opt/homebrew/include"
    export BOOST_LIBRARYDIR="/opt/homebrew/lib"
fi

clean () {
    ninja -C "${BUILD_DIR}" clean >&2
}

run_make () {
    name="$1"
    setting_id="$2"
    echo "# Building: ${name} (${setting_id})" >&2
    ninja -C "${BUILD_DIR}" "${setting_id}/${setting_id}_api_test" >&2
    ninja -C "${BUILD_DIR}" "${setting_id}/${setting_id}_bench" >&2
}

run_test () {
    name="$1"
    setting_id="$2"
    echo "# Testing: ${name} (${setting_id})" >&2
    "./${BUILD_DIR}/${setting_id}/${setting_id}_api_test" >&2
}

convert_to_us () {
    input="$1"
    if ! echo "${input}" | grep -Pq '^\d+(\.\d+)? (u|m|n)?s$'; then
        echo "unexpected format: '${input}'" >&2
        exit 1
    fi
    num="`echo "${input}" | cut -d ' ' -f 1`"
    unit="`echo "${input}" | cut -d ' ' -f 2`"
    if [ "$unit" = "ns" ]; then
        echo "scale=10; ${num} / 1000"  | bc
    elif [ "$unit" = "us" ]; then
        echo "${num}"
    elif [ "$unit" = "ms" ]; then
        echo "scale=10; ${num} * 1000"  | bc
    elif [ "$unit" = "s" ]; then
        echo "scale=10; ${num} * 1000 * 1000"  | bc
    fi
}

parse_csv_col_stats () {
    output="$1"
    column="$2"
    stats="`echo "${output}" | awk -F',' -v c="${column}" '
        NF >= 3 {
            x=$c+0
            if (n==0 || x<min) min=x
            if (n==0 || x>max) max=x
            a[n]=x
            sum+=x
            n++
        }
        END {
            if (n==0) { print "0,0,0,0,0"; exit }
            mean=sum/n
            ss=0
            for (i=0; i<n; i++) {
                d=a[i]-mean
                ss+=d*d
            }
            std=sqrt(ss/n)
            printf "%d,%.10f,%.10f,%.10f,%.10f\n", n, mean, std, min, max
        }'`"

    samples="`echo "${stats}" | cut -d',' -f1`"
    mean="`echo "${stats}" | cut -d',' -f2`"
    std_dev="`echo "${stats}" | cut -d',' -f3`"
    min="`echo "${stats}" | cut -d',' -f4`"
    max="`echo "${stats}" | cut -d',' -f5`"

    jq -c -n \
        --argjson "samples" "${samples}" \
        --argjson "mean_us" "${mean}" \
        --argjson "std_dev_us" "${std_dev}" \
        --argjson "min_us" "${min}" \
        --argjson "max_us" "${max}" \
        '$ARGS.named'
}

gather_metadata () {
    hostname="`uname -n`"
    user="`whoami`"
    if date --iso-8601=ns >/dev/null 2>&1; then
        timestamp="`date --iso-8601=ns`"
    else
        timestamp="`date -u '+%Y-%m-%dT%H:%M:%S.%NZ'`"
    fi
    if git_commit="`git rev-parse --verify HEAD 2>/dev/null`"; then
        :
    else
        git_commit="unknown"
    fi
    json="`jq -c -n \
        --arg "hostname" "${hostname}" \
        --arg "user" "${user}" \
        --arg "timestamp" "${timestamp}" \
        --arg "git_commit" "${git_commit}" \
        '$ARGS.named' \
        `"
    echo "${json}"
}

run_bench () {
    name="$1"
    setting_id="$2"
    echo "# Benching: ${name} (${setting_id})" >&2

    meta="`gather_metadata`"

    bench_out="`"./${BUILD_DIR}/${setting_id}/${setting_id}_bench" -i "${BENCH_ITER}"`"

    sk_size="`grep CRYPTO_SECRETKEYBYTES < "./${BUILD_DIR}/${setting_id}/api.h" | cut -d ' ' -f 3- | bc`"
    pk_size="`grep CRYPTO_PUBLICKEYBYTES < "./${BUILD_DIR}/${setting_id}/api.h" | cut -d ' ' -f 3`"
    sig_size="`grep CRYPTO_BYTES < "./${BUILD_DIR}/${setting_id}/api.h" | cut -d ' ' -f 3`"
    keygen_results="`parse_csv_col_stats "${bench_out}" 1`"
    sign_results="`parse_csv_col_stats "${bench_out}" 2`"
    verify_results="`parse_csv_col_stats "${bench_out}" 3`"

    json="`jq -c -n \
        --arg "implementation" "ref" \
        --arg "variant" "${name}" \
        --arg "setting_id" "${setting_id}" \
        --argjson "sig_size_bytes" "${sig_size}" \
        --argjson "sk_size_bytes" "${sk_size}" \
        --argjson "pk_size_bytes" "${pk_size}" \
        --argjson "keygen" "${keygen_results}" \
        --argjson "sign" "${sign_results}" \
        --argjson "verify" "${verify_results}" \
        --argjson "meta" "${meta}" \
        '$ARGS.named' \
        `"
    echo "${json}"
}

write_csv () {
    jsonl_file="$1"
    csv_file="$2"

    jq -r -s '
    [
      "implementation","variant","setting_id","sig_size_bytes","sk_size_bytes","pk_size_bytes",
      "keygen_samples","keygen_mean_us","keygen_std_dev_us","keygen_min_us","keygen_max_us",
      "sign_samples","sign_mean_us","sign_std_dev_us","sign_min_us","sign_max_us",
      "verify_samples","verify_mean_us","verify_std_dev_us","verify_min_us","verify_max_us",
      "hostname","user","timestamp","git_commit"
    ],
    (
      .[] as $x |
      [
        $x.implementation, $x.variant, $x.setting_id, $x.sig_size_bytes, $x.sk_size_bytes, $x.pk_size_bytes,
        $x.keygen.samples, $x.keygen.mean_us, $x.keygen.std_dev_us, $x.keygen.min_us, $x.keygen.max_us,
        $x.sign.samples, $x.sign.mean_us, $x.sign.std_dev_us, $x.sign.min_us, $x.sign.max_us,
        $x.verify.samples, $x.verify.mean_us, $x.verify.std_dev_us, $x.verify.min_us, $x.verify.max_us,
        $x.meta.hostname, $x.meta.user, $x.meta.timestamp, $x.meta.git_commit
      ]
    ) | @csv' "${jsonl_file}" > "${csv_file}"
}

bench_spec_variants () {
    : > "${RESULT_JSONL}"
    for faest_variant in $ALL; do
        setting_id="${!faest_variant}"
        run_make "${faest_variant}" "${setting_id}"
        if [ "${RUN_TESTS}" = "true" ]; then
            run_test "${faest_variant}" "${setting_id}"
        fi
        result="`run_bench "${faest_variant}" "${setting_id}"`"
        echo "${result}"
        echo "${result}" >> "${RESULT_JSONL}"
    done
    write_csv "${RESULT_JSONL}" "${RESULT_CSV}"
    echo "# Wrote JSONL: ${RESULT_JSONL}" >&2
    echo "# Wrote CSV:   ${RESULT_CSV}" >&2
}

mkdir -p "${BUILD_DIR}"
pushd "${BUILD_DIR}"
meson setup .. --buildtype release -Dbenchmarks=enabled -Dmarch-native=enabled -Dlynx_matrix_mode="${LYNX_MATRIX_MODE}" --reconfigure >&2 || \
meson setup .. --buildtype release -Dbenchmarks=enabled -Dmarch-native=enabled -Dlynx_matrix_mode="${LYNX_MATRIX_MODE}" >&2
popd

bench_spec_variants
