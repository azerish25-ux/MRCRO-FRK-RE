#pragma once
#include <array>
#include <cstdint>
#include <span>
namespace mfrk {
  enum class AdsrPhase : std::int32_t {
    idle, forced_maximum, attack, decay, sustain, release, retrigger
  };
  // Attack/decay/sustain/release, attack/decay/release curve, legato policy, mode.
  // Dirty-mask bits use this same order; inputs retain original ushort semantics.
  using AdsrControls = std::array<std::uint16_t,9>;
  struct AdsrState {
    AdsrPhase phase{};
    std::int32_t mode{}, accumulator{};
    std::int16_t current{}, output{};
    std::uint8_t retrigger_ticks{}, hold{}, pending_release{}, legato{}, trigger{};
    std::int16_t attack_time{}, decay_time{}, release_time{}, sustain{};
    std::int32_t attack_variant{}, decay_variant{}, release_variant{};
    std::int16_t decay_aux{}, release_aux{};
    std::array<std::int16_t,3> decay_constants{}, release_constants{};
    std::int16_t decay_dynamic{}, release_dynamic{};
    std::int32_t attack_coefficient{};
    std::int64_t attack_additive{};
    std::int32_t decay_coefficient{};
    std::int64_t decay_additive{};
    std::int32_t release_coefficient{};
    std::int64_t release_additive{};
  };
  // Native constructor explicitly defines phase/mode/value/output, prepared
  // times/sustain/variants, dynamic values and flags except retrigger_ticks.
  // Unwritten coefficient/auxiliary/constant/tick cells have deterministic zero
  // storage here, but these zeros are NOT claimed as original defaults.
  // Prepare every control once before processing; gate/step otherwise return false.
  class Adsr {
  public:
    // Prewarms both mathematical curves; construct outside the audio callback.
    Adsr() noexcept;
    void prepare(const AdsrControls&,std::uint16_t dirty_mask) noexcept;
    void reset() noexcept;
    // False means controls have not all been prepared by this portable API.
    bool gate(bool on) noexcept;
    bool step() noexcept;
    // Raw recovered flag only. Does not invent a gate-off dispatch when cleared.
    void set_hold_flag(bool held) noexcept {
      state_.hold=held;
    }
    [[nodiscard]] AdsrState snapshot() const noexcept {
      return state_;
    }
    [[nodiscard]] bool prepared() const noexcept {
      return prepared_mask_==0x1ff;
    }
    [[nodiscard]] const AdsrControls& controls() const noexcept {
      return controls_;
    }
    // Variant must be0 or1. Table bytes preserve native adjacent-field endpoints.
    [[nodiscard]] static std::span<const std::uint8_t,0x710> curve(std::uint32_t variant) noexcept;
  private:
    AdsrState state_{};
    AdsrControls controls_{};
    std::uint16_t prepared_mask_{};
    void release_coefficients() noexcept;
    void advance(std::int32_t,std::int64_t) noexcept;
    void sustain_value() noexcept;
friend struct AdsrTestAccess;
  };

}
