#include <array>
#include <iostream>
#include <vector>

#include "../faest.hpp"
#include "../api.hpp"
#include "test.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace faest;

// Single sign operation without benchmarking loops
template<typename P>
void profile_sign_once() {
    using FP = faest_scheme<P>;
    
    std::array<unsigned char, FP::CRYPTO_SECRETKEYBYTES> sk;
    std::array<unsigned char, FP::CRYPTO_PUBLICKEYBYTES> pk;
    
    // Keygen (setup, not profiled)
    FP::crypto_sign_keypair(pk.data(), sk.data());
    
    const std::string message =
        "This document describes and specifies the FAEST digital signature algorithm.";
    std::vector<unsigned char> signed_message(FP::CRYPTO_BYTES + message.size());
    unsigned long long signed_message_len;
    
    // Single sign operation - this is what we profile
    int result = FP::crypto_sign(signed_message.data(), &signed_message_len,
                               reinterpret_cast<const unsigned char*>(message.data()),
                               message.size(), sk.data());
    
    REQUIRE(result == 0);
    REQUIRE(signed_message_len == signed_message.size());
}

// Test cases for each Lynx instance
TEMPLATE_TEST_CASE("profile lynx 128 sign", "[profile][lynx]", lynx::lynx_128_s)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 128 fast sign", "[profile][lynx]", lynx::lynx_128_f)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 192 sign", "[profile][lynx]", lynx::lynx_192_s)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 192 fast sign", "[profile][lynx]", lynx::lynx_192_f)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 256 sign", "[profile][lynx]", lynx::lynx_256_s)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 256 fast sign", "[profile][lynx]", lynx::lynx_256_f)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 384 sign", "[profile][lynx]", lynx::lynx_384_s)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 384 fast sign", "[profile][lynx]", lynx::lynx_384_f)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 512 sign", "[profile][lynx]", lynx::lynx_512_s)
{
    profile_sign_once<TestType>();
}

TEMPLATE_TEST_CASE("profile lynx 512 fast sign", "[profile][lynx]", lynx::lynx_512_f)
{
    profile_sign_once<TestType>();
}

// Combined test for all instances (optional)
TEMPLATE_TEST_CASE("profile all lynx sign", "[profile][lynx]", ALL_LYNX_INSTANCES)
{
    profile_sign_once<TestType>();
}
