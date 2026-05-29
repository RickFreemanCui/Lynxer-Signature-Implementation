#include "faest.inc"

namespace faest
{

// clang-format off

template bool faest_unpack_secret_key(secret_key<lynx::lynx_128_s>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_128_f>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_192_s>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_192_f>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_256_s>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_256_f>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_384_s>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_384_f>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_512_s>*, const uint8_t*);
template bool faest_unpack_secret_key(secret_key<lynx::lynx_512_f>*, const uint8_t*);

template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_128_s>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_128_f>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_192_s>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_192_f>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_256_s>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_256_f>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_384_s>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_384_f>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_512_s>*);
template void faest_pack_public_key(uint8_t*, const public_key<lynx::lynx_512_f>*);

template void faest_unpack_public_key(public_key<lynx::lynx_128_s>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_128_f>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_192_s>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_192_f>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_256_s>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_256_f>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_384_s>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_384_f>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_512_s>*, const uint8_t*);
template void faest_unpack_public_key(public_key<lynx::lynx_512_f>*, const uint8_t*);

template bool faest_seckey<lynx::lynx_128_s>(const uint8_t*);
template bool faest_seckey<lynx::lynx_128_f>(const uint8_t*);
template bool faest_seckey<lynx::lynx_192_s>(const uint8_t*);
template bool faest_seckey<lynx::lynx_192_f>(const uint8_t*);
template bool faest_seckey<lynx::lynx_256_s>(const uint8_t*);
template bool faest_seckey<lynx::lynx_256_f>(const uint8_t*);
template bool faest_seckey<lynx::lynx_384_s>(const uint8_t*);
template bool faest_seckey<lynx::lynx_384_f>(const uint8_t*);
template bool faest_seckey<lynx::lynx_512_s>(const uint8_t*);
template bool faest_seckey<lynx::lynx_512_f>(const uint8_t*);

template bool faest_pubkey<lynx::lynx_128_s>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_128_f>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_192_s>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_192_f>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_256_s>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_256_f>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_384_s>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_384_f>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_512_s>(uint8_t*, const uint8_t*);
template bool faest_pubkey<lynx::lynx_512_f>(uint8_t*, const uint8_t*);

template bool faest_sign<lynx::lynx_128_s>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_128_f>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_192_s>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_192_f>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_256_s>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_256_f>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_384_s>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_384_f>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_512_s>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);
template bool faest_sign<lynx::lynx_512_f>(uint8_t*, const uint8_t*, size_t, const uint8_t*, const uint8_t*, size_t);

template bool faest_verify<lynx::lynx_128_s>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_128_f>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_192_s>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_192_f>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_256_s>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_256_f>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_384_s>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_384_f>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_512_s>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);
template bool faest_verify<lynx::lynx_512_f>(const uint8_t*, const uint8_t*, size_t, const uint8_t*);

// clang-format on

} // namespace faest
