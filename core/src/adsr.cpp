#include "mfrk/adsr.hpp"
namespace mfrk {
Adsr::Adsr() noexcept = default;
void Adsr::prepare(const AdsrControls& c,std::uint16_t m) noexcept { controls_=c;prepared_mask_|=(m&0x1ff); }
void Adsr::reset() noexcept {state_.accumulator=state_.current=0;state_.phase=AdsrPhase::idle;state_.hold=state_.pending_release=0;}
bool Adsr::gate(bool) noexcept {return prepared();}
bool Adsr::step() noexcept {return prepared();}
std::span<const std::uint8_t,0x710> Adsr::curve(std::uint32_t) noexcept {static constexpr std::array<std::uint8_t,0x710> empty{};return empty;}
}
