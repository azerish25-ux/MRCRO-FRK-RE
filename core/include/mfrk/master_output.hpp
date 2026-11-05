#pragma once
#include <array>
#include <cstdint>

namespace mfrk::master {
using StereoBlock = std::array<std::array<float,16>,2>;
struct DelayMemory { std::array<std::array<float,145>,2> samples; };
struct LimiterTestAccess;
class Limiter {
 public:
  explicit Limiter(DelayMemory& memory) noexcept;
  Limiter(const Limiter&)=delete;
  Limiter& operator=(const Limiter&)=delete;
  Limiter(Limiter&&)=delete;
  Limiter& operator=(Limiter&&)=delete;
  void reset() noexcept;
  // Source31b9e0, fixed16-frame/48000Hz two-channel master limiter.
  // Serialized audio-owner calls; finite inputs/controls/intermediates, IEEE
  // binary32, RN-even, gradual underflow and no fast-math/contraction.
  void process(const StereoBlock& input,StereoBlock& output) noexcept;
 private:
  friend struct LimiterTestAccess;
  DelayMemory& memory_;
  std::array<float,2> peak_{};
  std::array<float,2> gain_db_{};
  std::array<unsigned,2> valid_{};
  std::array<unsigned,2> write_{};
  // Original30e990 constructor bits, preserved rather than recomputed.
  float attack_,release_,peak_decay_,threshold_;
};

struct MasterTestAccess;
class Master {
 public:
  Master(const Master&)=delete;
  Master& operator=(const Master&)=delete;
  Master(Master&&)=delete;
  Master& operator=(Master&&)=delete;
  // The caller owns persistent ring contents and must outlive this object.
  // Raw constructor and post-owner-Init are distinct lifecycle points.
  // Reset preserves ring contents, allocation and coefficients.
  static Master admitted_zero_owner_arena(DelayMemory&) noexcept;
  static Master admitted_ready_owner(DelayMemory&) noexcept;
  // Prepared process-boundary controls: these are NOT normalized UI values.
  // Source30fb20's pregain conversion includes UCRTpowf(10,db*.05).
  void set_prepared_pre_gain(float value) noexcept;
  void set_prepared_post_gain(float value) noexcept;
  void set_mono(bool value) noexcept;
  void reset() noexcept;
  void process(const StereoBlock& input,StereoBlock& output) noexcept;
 private:
  friend struct MasterTestAccess;
  struct RawTag{};
  struct ReadyTag{};
  Master(DelayMemory&,RawTag) noexcept;
  Master(DelayMemory&,ReadyTag) noexcept;
  Limiter limiter_;
  float pre_gain_=1.0f,stored_post_gain_=0.0f;
  bool mono_=false;
  float mono_scale_=0.5f;
  float target_=0.0f,current_;
  float coefficient_,threshold_;
  std::array<float,16> gain_buffer_{};
};
} // namespace mfrk::master
