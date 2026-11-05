#pragma once
#include <array>
#include <cstddef>
#include <span>
namespace mfrk::audio {
// The source-profile engine generates 16 stereo frames at a time. The callback
// must fill both channels without blocking or allocating; false stops output.
using RenderBlock=bool(*)(void*,std::span<float,16>,std::span<float,16>) noexcept;
class BlockAdapter {
 RenderBlock render_;
 void* context_;
 std::array<float,16> left_{},right_{};
 std::size_t position_=16;
public:
 BlockAdapter(RenderBlock render,void* context) noexcept:render_(render),context_(context){}
 // Interleaved stereo. Unfilled output is zeroed if rendering fails. Odd-sized
 // spans are rejected. Caller owns sample-rate conversion and stream lifecycle.
 bool fill(std::span<float> interleaved) noexcept;
 void reset() noexcept {position_=16;}
};
}
