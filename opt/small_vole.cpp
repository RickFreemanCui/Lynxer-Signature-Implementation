#include "small_vole.inc"

namespace faest
{

// clang-format off

template void vole_sender<lynx::lynx_128_s>(unsigned int, const block_secpar<lynx::lynx_128_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_128_f>(unsigned int, const block_secpar<lynx::lynx_128_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_192_s>(unsigned int, const block_secpar<lynx::lynx_192_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_192_f>(unsigned int, const block_secpar<lynx::lynx_192_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_256_s>(unsigned int, const block_secpar<lynx::lynx_256_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_256_f>(unsigned int, const block_secpar<lynx::lynx_256_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_384_s>(unsigned int, const block_secpar<lynx::lynx_384_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_384_f>(unsigned int, const block_secpar<lynx::lynx_384_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_512_s>(unsigned int, const block_secpar<lynx::lynx_512_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);
template void vole_sender<lynx::lynx_512_f>(unsigned int, const block_secpar<lynx::lynx_512_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, vole_block* __restrict__);

template void vole_receiver<lynx::lynx_128_s>(unsigned int, const block_secpar<lynx::lynx_128_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_128_f>(unsigned int, const block_secpar<lynx::lynx_128_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_192_s>(unsigned int, const block_secpar<lynx::lynx_192_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_192_f>(unsigned int, const block_secpar<lynx::lynx_192_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_256_s>(unsigned int, const block_secpar<lynx::lynx_256_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_256_f>(unsigned int, const block_secpar<lynx::lynx_256_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_384_s>(unsigned int, const block_secpar<lynx::lynx_384_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_384_f>(unsigned int, const block_secpar<lynx::lynx_384_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_512_s>(unsigned int, const block_secpar<lynx::lynx_512_s::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver<lynx::lynx_512_f>(unsigned int, const block_secpar<lynx::lynx_512_f::secpar_v>* __restrict__, block128, uint32_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);

template void vole_receiver_apply_correction<lynx::lynx_128_s>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_128_f>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_192_s>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_192_f>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_256_s>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_256_f>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_384_s>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_384_f>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_512_s>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);
template void vole_receiver_apply_correction<lynx::lynx_512_f>(size_t, size_t, const vole_block* __restrict__, vole_block* __restrict__, const uint8_t* __restrict__);

// clang-format on

} // namespace faest
