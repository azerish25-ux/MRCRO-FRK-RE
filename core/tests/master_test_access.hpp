#pragma once
#include "mfrk/master_output.hpp"
#include <bit>
namespace mfrk::master {
struct LimiterTestAccess {
 static std::array<std::uint32_t,4> coefficients(const Limiter& x) noexcept{return {std::bit_cast<std::uint32_t>(x.attack_),std::bit_cast<std::uint32_t>(x.release_),std::bit_cast<std::uint32_t>(x.peak_decay_),std::bit_cast<std::uint32_t>(x.threshold_)};}
 static const auto& peak(const Limiter& x) noexcept{return x.peak_;}
 static const auto& gain_db(const Limiter& x) noexcept{return x.gain_db_;}
 static const auto& valid(const Limiter& x) noexcept{return x.valid_;}
 static const auto& write(const Limiter& x) noexcept{return x.write_;}
};
struct MasterTestAccess {
 static auto& limiter(Master& x) noexcept{return x.limiter_;}
 static std::array<std::uint32_t,4> smoother(const Master& x) noexcept{return {std::bit_cast<std::uint32_t>(x.target_),std::bit_cast<std::uint32_t>(x.current_),std::bit_cast<std::uint32_t>(x.coefficient_),std::bit_cast<std::uint32_t>(x.threshold_)};}
 static const auto& gains(const Master& x) noexcept{return x.gain_buffer_;}
 static std::array<std::uint32_t,4> controls(const Master& x) noexcept{return {std::bit_cast<std::uint32_t>(x.pre_gain_),std::bit_cast<std::uint32_t>(x.stored_post_gain_),std::uint32_t(x.mono_),std::bit_cast<std::uint32_t>(x.mono_scale_)};}
};
}
