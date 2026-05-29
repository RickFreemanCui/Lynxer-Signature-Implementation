#ifndef LYNX_MATRICES_HPP
#define LYNX_MATRICES_HPP

// Matrix handling for Lynx OWF
// Ported from lynx-sig/lynx_matrices.h

#include "parameters.hpp"
#include "hash.hpp"
#include "lynx_constants.hpp"
#include "lynx_l3_consts.hpp"
#include "hash.hpp"
#include <array>
#include <cstdint>
#include <cstring>

namespace faest
{
namespace lynx
{

// Maximum dimensions for matrix storage
constexpr std::size_t LYNX_MAX_LAMBDA = 512;
constexpr std::size_t LYNX_MAX_WORDS = LYNX_MAX_LAMBDA / 64;

// Helper: compute parity of 64-bit word
inline int parity_u64(uint64_t x)
{
    x ^= x >> 32;
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return static_cast<int>(x & 1);
}

// GF(2) matrix-vector multiplication
// M is stored row-major: M[i][k] = k-th 64-bit word of row i
template <std::size_t N, std::size_t W>
inline void lynx_L_words(const uint64_t M[][W], const std::array<uint64_t, W>& x, std::array<uint64_t, W>& res)
{
    res.fill(0);
    for (unsigned int i = 0; i < N; i++)
    {
        uint64_t dot = 0;
        for (unsigned int j = 0; j < W; ++j)
        {
            dot ^= (M[i][j] & x[j]);
        }
        if (parity_u64(dot))
        {
            res[i / 64] ^= UINT64_C(1) << (i % 64);
        }
    }
}

template <secpar S>
inline poly_secpar<S> lynx_L(const uint64_t M[][secpar_to_bits(S) / 64], const poly_secpar<S>& x)
{
    constexpr std::size_t N = secpar_to_bits(S);
    constexpr std::size_t W = N / 64;
    std::array<uint64_t, W> in_words{};
    std::array<uint64_t, W> out_words{};
    x.store(in_words.data());
    lynx_L_words<N, W>(M, in_words, out_words);
    return poly_secpar<S>::load(out_words.data());
}

// Matrix structure for Lynx OWF
template <secpar S>
struct lynx_matrices;

template <>
struct lynx_matrices<secpar::s128>
{
    static constexpr std::size_t N = 128;
    static constexpr std::size_t words = 2;
    std::array<std::array<uint64_t, words>, N> L0{}, L1{}, L2{}, L3{};
    std::array<uint8_t, 16> C0{}, C1{};

    void clear()
    {
        L0.fill({});
        L1.fill({});
        L2.fill({});
        L3.fill({});
        C0.fill({});
        C1.fill({});
    }
};

template <>
struct lynx_matrices<secpar::s192>
{
    static constexpr std::size_t N = 192;
    static constexpr std::size_t words = 3;
    std::array<std::array<uint64_t, words>, N> L0{}, L1{}, L2{}, L3{};
    std::array<uint8_t, 24> C0{}, C1{};

    void clear()
    {
        L0.fill({});
        L1.fill({});
        L2.fill({});
        L3.fill({});
        C0.fill({});
        C1.fill({});
    }
};

template <>
struct lynx_matrices<secpar::s256>
{
    static constexpr std::size_t N = 256;
    static constexpr std::size_t words = 4;
    std::array<std::array<uint64_t, words>, N> L0{}, L1{}, L2{}, L3{};
    std::array<uint8_t, 32> C0{}, C1{}, C2{};

    void clear()
    {
        L0.fill({});
        L1.fill({});
        L2.fill({});
        L3.fill({});
        C0.fill({});
        C1.fill({});
        C2.fill({});
    }
};

template <>
struct lynx_matrices<secpar::s384>
{
    static constexpr std::size_t N = 384;
    static constexpr std::size_t words = 6;
    std::array<std::array<uint64_t, words>, N> L0{}, L1{}, L2{}, L3{};
    std::array<uint8_t, 48> C0{}, C1{}, C2{};

    void clear()
    {
        L0.fill({});
        L1.fill({});
        L2.fill({});
        L3.fill({});
        C0.fill({});
        C1.fill({});
        C2.fill({});
    }
};

template <>
struct lynx_matrices<secpar::s512>
{
    static constexpr std::size_t N = 512;
    static constexpr std::size_t words = 8;
    std::array<std::array<uint64_t, words>, N> L0{}, L1{}, L2{}, L3{};
    std::array<uint8_t, 64> C0{}, C1{}, C2{};

    void clear()
    {
        L0.fill({});
        L1.fill({});
        L2.fill({});
        L3.fill({});
        C0.fill({});
        C1.fill({});
        C2.fill({});
    }
};

// Helper functions for LU-sampling bit masks
inline uint64_t lower_mask(unsigned int bit)
{
    if (bit == 0)
        return 0;
    return (UINT64_C(1) << bit) - 1;
}

inline uint64_t upper_mask(unsigned int bit)
{
    if (bit == 63)
        return 0;
    return ~((UINT64_C(1) << (bit + 1)) - 1);
}

// Squeeze a little-endian uint64_t from the SHAKE context
inline uint64_t squeeze_u64(Keccak_HashInstance& ctx)
{
    uint8_t tmp[8];
    Keccak_HashSqueeze(&ctx, tmp, 64);
    return static_cast<uint64_t>(tmp[0]) | (static_cast<uint64_t>(tmp[1]) << 8) |
           (static_cast<uint64_t>(tmp[2]) << 16) | (static_cast<uint64_t>(tmp[3]) << 24) |
           (static_cast<uint64_t>(tmp[4]) << 32) | (static_cast<uint64_t>(tmp[5]) << 40) |
           (static_cast<uint64_t>(tmp[6]) << 48) | (static_cast<uint64_t>(tmp[7]) << 56);
}

// Sample a lower triangular matrix with 1s on diagonal
template <std::size_t N, std::size_t W>
void sample_lower_triangular(Keccak_HashInstance& ctx,
                             std::array<std::array<uint64_t, W>, N>& L)
{
    for (unsigned int r = 0; r < N; ++r)
    {
        const unsigned int diag_word = r / 64;
        const unsigned int diag_bit = r % 64;
        for (unsigned int w = 0; w < diag_word; ++w)
            L[r][w] = squeeze_u64(ctx);
        L[r][diag_word] = (squeeze_u64(ctx) & lower_mask(diag_bit)) | (UINT64_C(1) << diag_bit);
        for (unsigned int w = diag_word + 1; w < W; ++w)
            L[r][w] = 0;
    }
}

// Sample an upper triangular matrix with 1s on diagonal
template <std::size_t N, std::size_t W>
void sample_upper_triangular(Keccak_HashInstance& ctx,
                             std::array<std::array<uint64_t, W>, N>& U)
{
    for (unsigned int r = 0; r < N; ++r)
    {
        const unsigned int diag_word = r / 64;
        const unsigned int diag_bit = r % 64;
        for (unsigned int w = 0; w < diag_word; ++w)
            U[r][w] = 0;
        U[r][diag_word] = (squeeze_u64(ctx) & upper_mask(diag_bit)) | (UINT64_C(1) << diag_bit);
        for (unsigned int w = diag_word + 1; w < W; ++w)
            U[r][w] = squeeze_u64(ctx);
    }
}

// GF(2) matrix multiplication: C = A * B
template <std::size_t N, std::size_t W>
void matrix_mul_gf2(const std::array<std::array<uint64_t, W>, N>& A,
                    const std::array<std::array<uint64_t, W>, N>& B,
                    std::array<std::array<uint64_t, W>, N>& C)
{
    for (unsigned int r = 0; r < N; ++r)
        C[r].fill(0);

    for (unsigned int r = 0; r < N; ++r)
    {
        for (unsigned int w = 0; w < W; ++w)
        {
            uint64_t x = A[r][w];
            while (x)
            {
                const unsigned int bit = static_cast<unsigned int>(__builtin_ctzll(x));
                const unsigned int c = w * 64 + bit;
                if (c < N)
                {
                    for (unsigned int j = 0; j < W; ++j)
                        C[r][j] ^= B[c][j];
                }
                x &= (x - 1);
            }
        }
    }
}

// Sample a random invertible matrix M = upper * lower
template <std::size_t N, std::size_t W>
void sample_matrix(Keccak_HashInstance& ctx,
                   std::array<std::array<uint64_t, W>, N>& M)
{
    std::array<std::array<uint64_t, W>, N> lower_mat, upper_mat;
    sample_lower_triangular<N, W>(ctx, lower_mat);
    sample_upper_triangular<N, W>(ctx, upper_mat);
    matrix_mul_gf2<N, W>(upper_mat, lower_mat, M);
}

// Initialize matrices from seed using LU-sampling
// Matches the reference lynx-sig/lynx_matrices.c lynx_init()
template <secpar S>
void lynx_init(lynx_matrices<S>& mats, const uint8_t* seed, std::size_t seed_len)
{
    constexpr unsigned int lambda = static_cast<unsigned int>(secpar_to_bits(S));
    constexpr unsigned int bytes = lambda / 8;
    constexpr unsigned int W = lynx_matrices<S>::words;

    // Initialize SHAKE-based RNG with domain separator
    Keccak_HashInstance ctx;
    keccak_shake_init(&ctx, S);

    // Domain separation: "LYNX-MATGEN-v1" || lambda_le || seed_len_le || seed
    const uint8_t domain[] = "LYNX-MATGEN-v1";
    Keccak_HashUpdate(&ctx, domain, (sizeof(domain) - 1) * 8);

    const uint8_t lambda_le[2] = {static_cast<uint8_t>(lambda & 0xff),
                                  static_cast<uint8_t>((lambda >> 8) & 0xff)};
    Keccak_HashUpdate(&ctx, lambda_le, sizeof(lambda_le) * 8);

    const uint8_t seed_len_le[2] = {static_cast<uint8_t>(seed_len & 0xff),
                                    static_cast<uint8_t>((seed_len >> 8) & 0xff)};
    Keccak_HashUpdate(&ctx, seed_len_le, sizeof(seed_len_le) * 8);

    Keccak_HashUpdate(&ctx, seed, seed_len * 8);
    Keccak_HashFinal(&ctx, NULL);

    // Sample 3 matrices via LU decomposition (L3 is a fixed constant)
    sample_matrix<lambda, W>(ctx, mats.L0);
    sample_matrix<lambda, W>(ctx, mats.L1);
    sample_matrix<lambda, W>(ctx, mats.L2);
    mats.L3 = lynx_l3_matrix<S>();

    // Squeeze constants
    Keccak_HashSqueeze(&ctx, mats.C0.data(), bytes * 8);
    Keccak_HashSqueeze(&ctx, mats.C1.data(), bytes * 8);
    if constexpr (S == secpar::s256 || S == secpar::s384 || S == secpar::s512)
    {
        Keccak_HashSqueeze(&ctx, mats.C2.data(), bytes * 8);
    }
}

} // namespace lynx
} // namespace faest

#endif
