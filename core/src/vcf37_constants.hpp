// Exact float32 constants from original raw VCF routines; see reconstruction evidence.
#pragma once
#include <array>
#include <bit>
#include <cstdint>
namespace mfrk::detail {
template<std::uint64_t Address> constexpr float vcf_constant() {
    if constexpr (Address == 0x180563000ull) return std::bit_cast<float>(std::uint32_t{0x3f800000u});
    else if constexpr (Address == 0x18055a00cull) return std::bit_cast<float>(std::uint32_t{0x3f8126e9u});
    else if constexpr (Address == 0x18055a500ull) return std::bit_cast<float>(std::uint32_t{0x477fff00u});
    else if constexpr (Address == 0x18055a38cull) return std::bit_cast<float>(std::uint32_t{0x434b0000u});
    else if constexpr (Address == 0x18055a518ull) return std::bit_cast<float>(std::uint32_t{0x48060200u});
    else if constexpr (Address == 0x180559ec4ull) return std::bit_cast<float>(std::uint32_t{0x3ee81167u});
    else if constexpr (Address == 0x18055a4c8ull) return std::bit_cast<float>(std::uint32_t{0x46d3525cu});
    else if constexpr (Address == 0x180554834ull) return std::bit_cast<float>(std::uint32_t{0x2a1d4952u});
    else if constexpr (Address == 0x1805587e8ull) return std::bit_cast<float>(std::uint32_t{0x399d4952u});
    else if constexpr (Address == 0x18055a220ull) return std::bit_cast<float>(std::uint32_t{0x411de9e7u});
    else if constexpr (Address == 0x180559ea4ull) return std::bit_cast<float>(std::uint32_t{0x3ed67751u});
    else if constexpr (Address == 0x18055a028ull) return std::bit_cast<float>(std::uint32_t{0x3f860a92u});
    else if constexpr (Address == 0x18055f7d4ull) return std::bit_cast<float>(std::uint32_t{0xc0490fdbu});
    else if constexpr (Address == 0x180559e8cull) return std::bit_cast<float>(std::uint32_t{0x3ec22011u});
    else if constexpr (Address == 0x180559da0ull) return std::bit_cast<float>(std::uint32_t{0x3dbf74c2u});
    else if constexpr (Address == 0x180559f68ull) return std::bit_cast<float>(std::uint32_t{0x3f37cb7du});
    else if constexpr (Address == 0x180563110ull) return std::bit_cast<float>(std::uint32_t{0x7fffffffu});
    else if constexpr (Address == 0x18055a2c4ull) return std::bit_cast<float>(std::uint32_t{0x41dfb2fbu});
    else if constexpr (Address == 0x180554a1cull) return std::bit_cast<float>(std::uint32_t{0x3518e071u});
    else if constexpr (Address == 0x180554890ull) return std::bit_cast<float>(std::uint32_t{0x31ea32b3u});
    else if constexpr (Address == 0x180559e28ull) return std::bit_cast<float>(std::uint32_t{0x3e5707a5u});
    else if constexpr (Address == 0x180559c34ull) return std::bit_cast<float>(std::uint32_t{0x3c83126fu});
    else if constexpr (Address == 0x18055a1e8ull) return std::bit_cast<float>(std::uint32_t{0x40d9999au});
    else if constexpr (Address == 0x18055a048ull) return std::bit_cast<float>(std::uint32_t{0x3f93a5e3u});
    else if constexpr (Address == 0x180559ff4ull) return std::bit_cast<float>(std::uint32_t{0x3f800000u});
    else if constexpr (Address == 0x18055a10cull) return std::bit_cast<float>(std::uint32_t{0x40000000u});
    else if constexpr (Address == 0x18055a0a8ull) return std::bit_cast<float>(std::uint32_t{0x3fc12c15u});
    else if constexpr (Address == 0x18055a0a0ull) return std::bit_cast<float>(std::uint32_t{0x3fbed3ebu});
    else if constexpr (Address == 0x180559d90ull) return std::bit_cast<float>(std::uint32_t{0x3dafa20fu});
    else if constexpr (Address == 0x180559f14ull) return std::bit_cast<float>(std::uint32_t{0x3f0f81cau});
    else if constexpr (Address == 0x18055a064ull) return std::bit_cast<float>(std::uint32_t{0x3fa45520u});
    else if constexpr (Address == 0x180559d08ull) return std::bit_cast<float>(std::uint32_t{0x3d424e13u});
    else if constexpr (Address == 0x180559db0ull) return std::bit_cast<float>(std::uint32_t{0x3dd361d9u});
    else if constexpr (Address == 0x180559de4ull) return std::bit_cast<float>(std::uint32_t{0x3e0a6fbdu});
    else if constexpr (Address == 0x18055f764ull) return std::bit_cast<float>(std::uint32_t{0xbf800000u});
    else if constexpr (Address == 0x18055f71cull) return std::bit_cast<float>(std::uint32_t{0xbdd361d9u});
    else if constexpr (Address == 0x18055f728ull) return std::bit_cast<float>(std::uint32_t{0xbe0a6fbdu});
    else if constexpr (Address == 0x18055f7a8ull) return std::bit_cast<float>(std::uint32_t{0xc0000000u});
    else if constexpr (Address == 0x18055f740ull) return std::bit_cast<float>(std::uint32_t{0xbf0f81cau});
    else if constexpr (Address == 0x18055f70cull) return std::bit_cast<float>(std::uint32_t{0xbd424e13u});
    else if constexpr (Address == 0x18055f77cull) return std::bit_cast<float>(std::uint32_t{0xbfbed3ebu});
    else if constexpr (Address == 0x18055f780ull) return std::bit_cast<float>(std::uint32_t{0xbfc12c15u});
    else if constexpr (Address == 0x180559f50ull) return std::bit_cast<float>(std::uint32_t{0x3f317218u});
    else if constexpr (Address == 0x18055f734ull) return std::bit_cast<float>(std::uint32_t{0xbe800000u});
    else if constexpr (Address == 0x18055a2e4ull) return std::bit_cast<float>(std::uint32_t{0x42000000u});
    else if constexpr (Address == 0x18055a3acull) return std::bit_cast<float>(std::uint32_t{0x43af0000u});
    else if constexpr (Address == 0x18055a42cull) return std::bit_cast<float>(std::uint32_t{0x44ff8c5cu});
    else if constexpr (Address == 0x18055492cull) return std::bit_cast<float>(std::uint32_t{0x345ee99du});
    else if constexpr (Address == 0x180558064ull) return std::bit_cast<float>(std::uint32_t{0x392f92cfu});
    else if constexpr (Address == 0x180559fc4ull) return std::bit_cast<float>(std::uint32_t{0x3f74c8cfu});
    else if constexpr (Address == 0x18055a120ull) return std::bit_cast<float>(std::uint32_t{0x40078831u});
    else if constexpr (Address == 0x18055a1c0ull) return std::bit_cast<float>(std::uint32_t{0x40c00000u});
    else if constexpr (Address == 0x180557a20ull) return std::bit_cast<float>(std::uint32_t{0x38e32be1u});
    else if constexpr (Address == 0x180559b50ull) return std::bit_cast<float>(std::uint32_t{0x3c13dafdu});
    else if constexpr (Address == 0x180559f74ull) return std::bit_cast<float>(std::uint32_t{0x3f38b2c7u});
    else if constexpr (Address == 0x180559ddcull) return std::bit_cast<float>(std::uint32_t{0x3e08e42du});
    else if constexpr (Address == 0x1805597f0ull) return std::bit_cast<float>(std::uint32_t{0x3b6e215au});
    else if constexpr (Address == 0x180559d70ull) return std::bit_cast<float>(std::uint32_t{0x3d90fb98u});
    else if constexpr (Address == 0x180559e80ull) return std::bit_cast<float>(std::uint32_t{0x3eb8d39fu});
    else if constexpr (Address == 0x180559f20ull) return std::bit_cast<float>(std::uint32_t{0x3f112a7du});
    else if constexpr (Address == 0x18055f7e0ull) return std::bit_cast<float>(std::uint32_t{0xc0980000u});
    else if constexpr (Address == 0x18055f7d0ull) return std::bit_cast<float>(std::uint32_t{0xc0400000u});
    else if constexpr (Address == 0x180559a90ull) return std::bit_cast<float>(std::uint32_t{0x3bed3009u});
    else if constexpr (Address == 0x180559d94ull) return std::bit_cast<float>(std::uint32_t{0x3db06028u});
    else if constexpr (Address == 0x180559e84ull) return std::bit_cast<float>(std::uint32_t{0x3ebbb500u});
    else if constexpr (Address == 0x180559f24ull) return std::bit_cast<float>(std::uint32_t{0x3f115302u});
    else if constexpr (Address == 0x180559e1cull) return std::bit_cast<float>(std::uint32_t{0x3e467283u});
    else if constexpr (Address == 0x180559b24ull) return std::bit_cast<float>(std::uint32_t{0x3c00a0acu});
    else if constexpr (Address == 0x180559f4cull) return std::bit_cast<float>(std::uint32_t{0x3f31645eu});
    else if constexpr (Address == 0x180559d18ull) return std::bit_cast<float>(std::uint32_t{0x3d61dcf4u});
    else if constexpr (Address == 0x18055f804ull) return std::bit_cast<float>(std::uint32_t{0xc0d00000u});
    else if constexpr (Address == 0x180559d6cull) return std::bit_cast<float>(std::uint32_t{0x3d8ff54au});
    else if constexpr (Address == 0x180558dbcull) return std::bit_cast<float>(std::uint32_t{0x3a016c91u});
    else if constexpr (Address == 0x180559ed8ull) return std::bit_cast<float>(std::uint32_t{0x3efdc4fau});
    else if constexpr (Address == 0x18055992cull) return std::bit_cast<float>(std::uint32_t{0x3b8df5a5u});
    else if constexpr (Address == 0x18055f81cull) return std::bit_cast<float>(std::uint32_t{0xc1f00000u});
    else if constexpr (Address == 0x18055a090ull) return std::bit_cast<float>(std::uint32_t{0x3fb8635fu});
    else if constexpr (Address == 0x180559c3cull) return std::bit_cast<float>(std::uint32_t{0x3c84e40fu});
    else if constexpr (Address == 0x18055f82cull) return std::bit_cast<float>(std::uint32_t{0xc2fc0000u});
    else if constexpr (Address == 0x180559d80ull) return std::bit_cast<float>(std::uint32_t{0x3da2b23fu});
    else if constexpr (Address == 0x180559e30ull) return std::bit_cast<float>(std::uint32_t{0x3e68de81u});
    else if constexpr (Address == 0x180559dc0ull) return std::bit_cast<float>(std::uint32_t{0x3de8de81u});
    else if constexpr (Address == 0x180559ea8ull) return std::bit_cast<float>(std::uint32_t{0x3ed75370u});
    else if constexpr (Address == 0x180559f40ull) return std::bit_cast<float>(std::uint32_t{0x3f2aaaabu});
    else if constexpr (Address == 0x180559ee8ull) return std::bit_cast<float>(std::uint32_t{0x3f000000u});
    else if constexpr (Address == 0x18055a304ull) return std::bit_cast<float>(std::uint32_t{0x42480000u});
    else if constexpr (Address == 0x18055a2a4ull) return std::bit_cast<float>(std::uint32_t{0x41c80000u});
    else if constexpr (Address == 0x180559fd0ull) return std::bit_cast<float>(std::uint32_t{0x3f7ae148u});
    else if constexpr (Address == 0x18055a490ull) return std::bit_cast<float>(std::uint32_t{0x461c4000u});
    else if constexpr (Address == 0x18055a4c4ull) return std::bit_cast<float>(std::uint32_t{0x46cb2000u});
    else if constexpr (Address == 0x1805591fcull) return std::bit_cast<float>(std::uint32_t{0x3a83126fu});
    else if constexpr (Address == 0x18055a308ull) return std::bit_cast<float>(std::uint32_t{0x4248000du});
    else if constexpr (Address == 0x180559fa8ull) return std::bit_cast<float>(std::uint32_t{0x3f666666u});
    else if constexpr (Address == 0x1805573c0ull) return std::bit_cast<float>(std::uint32_t{0x3895cbecu});
    else if constexpr (Address == 0x18055a300ull) return std::bit_cast<float>(std::uint32_t{0x4247fffdu});
    else if constexpr (Address == 0x180559f88ull) return std::bit_cast<float>(std::uint32_t{0x3f451eb8u});
    else if constexpr (Address == 0x180556530ull) return std::bit_cast<float>(std::uint32_t{0x37d5f31au});
    else if constexpr (Address == 0x1805566a8ull) return std::bit_cast<float>(std::uint32_t{0x37e98896u});
    else if constexpr (Address == 0x18055484cull) return std::bit_cast<float>(std::uint32_t{0x2dc18399u});
    else if constexpr (Address == 0x180554854ull) return std::bit_cast<float>(std::uint32_t{0x2e4eb54cu});
    else if constexpr (Address == 0x18055a558ull) return std::bit_cast<float>(std::uint32_t{0x4e3f4bbdu});
    else if constexpr (Address == 0x18059cba8ull) return std::bit_cast<float>(std::uint32_t{0x2ddfa232u});
    else if constexpr (Address == 0x180550cc0ull) return std::bit_cast<float>(std::uint32_t{0x6f747543u});
    else if constexpr (Address == 0x18055f9d0ull) return std::bit_cast<float>(std::uint32_t{0x3f800000u});
    else if constexpr (Address == 0x18055f870ull) return std::bit_cast<float>(std::uint32_t{0x0u});
    else if constexpr (Address == 0x180550ce0ull) return std::bit_cast<float>(std::uint32_t{0x6f736552u});
    else if constexpr (Address == 0x180550cd0ull) return std::bit_cast<float>(std::uint32_t{0x746c6946u});
    else if constexpr (Address == 0x18055a178ull) return std::bit_cast<float>(std::uint32_t{0x40400000u});
    else if constexpr (Address == 0x18055f8a0ull) return std::bit_cast<float>(std::uint32_t{0x3f800000u});
    else if constexpr (Address == 0x180550cf8ull) return std::bit_cast<float>(std::uint32_t{0x73696f4eu});
    else if constexpr (Address == 0x180552d80ull) return std::bit_cast<float>(std::uint32_t{0x3934464du});
    else if constexpr (Address == 0x18055f880ull) return std::bit_cast<float>(std::uint32_t{0x3f800000u});
    else if constexpr (Address == 0x180553c88ull) return std::bit_cast<float>(std::uint32_t{0x80339f50u});
    else if constexpr (Address == 0x180553cf8ull) return std::bit_cast<float>(std::uint32_t{0x8033b610u});
    else if constexpr (Address == 0x180552d98ull) return std::bit_cast<float>(std::uint32_t{0x80322280u});
    else if constexpr (Address == 0x18055f7c0ull) return std::bit_cast<float>(std::uint32_t{0x0u});
    else if constexpr (Address == 0x18055a0f0ull) return std::bit_cast<float>(std::uint32_t{0x0u});
    else if constexpr (Address == 0x18055f818ull) return std::bit_cast<float>(std::uint32_t{0xc1490fdbu});
    else if constexpr (Address == 0x180563140ull) return std::bit_cast<float>(std::uint32_t{0x80000000u});
    else if constexpr (Address == 0x18055a238ull) return std::bit_cast<float>(std::uint32_t{0x41280000u});
    else { static_assert(Address == 0, "Unknown VCF constant"); return 0.0f; }
}
inline constexpr std::array<float,27> kVcf37CutoffTable = {
    std::bit_cast<float>(std::uint32_t{0x4159999au}),
    std::bit_cast<float>(std::uint32_t{0x419ee148u}),
    std::bit_cast<float>(std::uint32_t{0x41eb0a3du}),
    std::bit_cast<float>(std::uint32_t{0x4220cccdu}),
    std::bit_cast<float>(std::uint32_t{0x42563333u}),
    std::bit_cast<float>(std::uint32_t{0x428ff5c3u}),
    std::bit_cast<float>(std::uint32_t{0x42c14ccdu}),
    std::bit_cast<float>(std::uint32_t{0x430263d7u}),
    std::bit_cast<float>(std::uint32_t{0x432e4000u}),
    std::bit_cast<float>(std::uint32_t{0x436b7333u}),
    std::bit_cast<float>(std::uint32_t{0x439ebae1u}),
    std::bit_cast<float>(std::uint32_t{0x43d335c3u}),
    std::bit_cast<float>(std::uint32_t{0x440ecae1u}),
    std::bit_cast<float>(std::uint32_t{0x4440870au}),
    std::bit_cast<float>(std::uint32_t{0x4480499au}),
    std::bit_cast<float>(std::uint32_t{0x44ad2ccdu}),
    std::bit_cast<float>(std::uint32_t{0x44e96ccdu}),
    std::bit_cast<float>(std::uint32_t{0x451dcb33u}),
    std::bit_cast<float>(std::uint32_t{0x4554d800u}),
    std::bit_cast<float>(std::uint32_t{0x45909ccdu}),
    std::bit_cast<float>(std::uint32_t{0x45c49d9au}),
    std::bit_cast<float>(std::uint32_t{0x46130a66u}),
    std::bit_cast<float>(std::uint32_t{0x466d2400u}),
    std::bit_cast<float>(std::uint32_t{0x46cabc00u}),
    std::bit_cast<float>(std::uint32_t{0x46cabc00u}),
    std::bit_cast<float>(std::uint32_t{0x46cabc00u}),
    std::bit_cast<float>(std::uint32_t{0x46cabc00u}),
};
inline constexpr std::array<float,7> kVcf37ResonanceTable = {
    std::bit_cast<float>(std::uint32_t{0x3f666666u}),
    std::bit_cast<float>(std::uint32_t{0x3f6b851fu}),
    std::bit_cast<float>(std::uint32_t{0x3f70a3d7u}),
    std::bit_cast<float>(std::uint32_t{0x3f75c28fu}),
    std::bit_cast<float>(std::uint32_t{0x3f78d4feu}),
    std::bit_cast<float>(std::uint32_t{0x3f800000u}),
    std::bit_cast<float>(std::uint32_t{0x3f800000u}),
};
}  // namespace mfrk::detail
