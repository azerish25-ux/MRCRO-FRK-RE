#include "mfrk/fir_resampler.hpp"
#include <algorithm>
namespace mfrk {
FirResampler::FirResampler(FilterOversampling factor,std::size_t frames):factor_(static_cast<std::size_t>(factor)),maximum_frames_(frames),phase_length_(0){}
void FirResampler::reset() noexcept{}
void FirResampler::upsample(std::span<const float>,std::span<float> output) noexcept{std::fill(output.begin(),output.end(),0.0f);}
void FirResampler::downsample(std::span<const float>,std::span<float> output) noexcept{std::fill(output.begin(),output.end(),0.0f);}
std::size_t FirResampler::factor() const noexcept{return factor_;}
std::uint32_t FirResampler::latency_frames() const noexcept{return 0;}
}
