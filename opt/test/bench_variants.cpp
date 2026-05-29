#include <array>

#include "all.inc"
#include "api.hpp"
#include "test.hpp"

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

TEMPLATE_TEST_CASE("bench variants", "[.][bench][faest-variants]", ALL_LYNX_INSTANCES)
{
    using P = TestType;
    using FP = faest_scheme<P>;

    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;

    BENCHMARK("keygen") { return FP::crypto_sign_keypair(pk.data(), sk.data()); };

    const auto message = benchmark_message_for_secpar<P>();
    std::vector<unsigned char> signed_message(FP::CRYPTO_BYTES + message.size());
    unsigned long long signed_message_len = 0;

    BENCHMARK("sign")
    {
        return FP::crypto_sign(signed_message.data(), &signed_message_len,
                               message.data(), message.size(), sk.data());
    };

    REQUIRE(signed_message_len == signed_message.size());
    std::vector<unsigned char> opened_message(message.size());
    unsigned long long opened_message_len = 0;

    BENCHMARK("verify")
    {
        return FP::crypto_sign_open(opened_message.data(), &opened_message_len,
                                    signed_message.data(), signed_message_len, pk.data());
    };

    REQUIRE(opened_message_len == opened_message.size());
    REQUIRE(opened_message ==
            message);

    std::cout << "{\n"
              << R"(    "sk_size": )" << FP::CRYPTO_SECRETKEYBYTES << ",\n"
              << R"(    "pk_size": )" << FP::CRYPTO_PUBLICKEYBYTES << ",\n"
              << R"(    "sig_size": )" << FP::CRYPTO_BYTES << ",\n"
              << R"(    "secpar": )" << P::secpar_bits << ",\n"
              << R"(    "tau": )" << P::tau_v << ",\n"
              << R"(    "delta_bits": )" << P::bavc_t::delta_bits_v << ",\n"
              << R"(    "open_threshold": )" << P::bavc_t::opening_seeds_threshold_v << "\n"
              << "}";
}
