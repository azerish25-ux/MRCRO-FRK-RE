#pragma once
#include "mfrk/basic_waves.hpp"
#include "mfrk/superwave.hpp"
#include "mfrk/harmo.hpp"
#include "mfrk/karplus.hpp"
#include "mfrk/virtual_analog_oscillator.hpp"
#include "mfrk/virtual_analog_controls.hpp"
#include "mfrk/waveshaper.hpp"
#include "mfrk/bass.hpp"
#include "mfrk/pitch_update.hpp"
#include "mfrk/slot_pitch.hpp"
#include <array>
#include <bit>
#include <span>
namespace mfrk::core_bank {
// Source model identity, not a UI type enum. Osc2 Bass uses native type12.
enum class Model:unsigned {basic,super,harmo,karplus,va,waveshaper,bass};
struct Tables {mfrk::BasicWaveTables basic;mfrk::harmo::Tables harmo;mfrk::karplus::Tables karplus;mfrk::waveshaper::Tables waveshaper;};
class Side {
 mfrk::karplus::Memory memory_{};
 mfrk::BasicWaves basic_;
 mfrk::SuperWave super_;
 mfrk::harmo::Engine harmo_;
 mfrk::karplus::Engine karplus_;
 mfrk::VirtualAnalogOscillator va_;
 mfrk::VirtualAnalogControlState va_controls_{};
 mfrk::waveshaper::Engine waveshaper_;
 mfrk::Bass bass_;
 std::array<std::array<std::uint16_t,3>,7> words_{};
 std::array<std::array<std::uint8_t,3>,7> dirty_{};
 std::array<std::uint8_t,7> active_{1,0,0,0,0,0,0};
 std::uint64_t identity_;
 static constexpr unsigned index(Model m) noexcept{return static_cast<unsigned>(m);}
public:
 // Tables and shared random state must outlive the side. Each side owns the
 // shared slot scratch reused by its Karplus, Harmo and virtual-analog models.
 Side(const Tables&t,mfrk::karplus::SharedRandom&r,std::uint64_t identity)
 :basic_(t.basic),super_(t.basic,mfrk::SuperWaveState::admitted({0,0,0},{0,0,0})),
 harmo_(mfrk::harmo::Engine::admitted_zero_arena_uninitialized(t.harmo,std::span(memory_.samples).first<2048>())),
 karplus_(mfrk::karplus::Engine::admitted_zero_arena(t.karplus,r,memory_)),
 va_(std::as_writable_bytes(std::span(memory_.samples)).first<128>()),
 waveshaper_(mfrk::waveshaper::Engine::admitted_zero_arena(t.waveshaper)),
 bass_(mfrk::BassState::admitted({0,0,0},{0,0,0})),identity_(identity){}
 Side(const Side&)=delete;Side&operator=(const Side&)=delete;
 const auto& memory() const noexcept{return memory_.samples;}
 void owner_policy(std::uint8_t x)noexcept {basic_.set_trigger_suppressed(x);super_.set_trigger_suppressed(x);harmo_.set_owner_policy_byte(x);bass_.set_trigger_suppressed(x);}
 std::uint8_t active(Model m)const noexcept{return active_[index(m)];}
 void active(Model m,std::uint8_t x)noexcept {active_[index(m)]=x;if(m==Model::karplus)karplus_.set_flag(x);}
 std::uint16_t word(Model m,unsigned i)const noexcept{return words_[index(m)][i];}
 void word(Model m,unsigned i,std::uint16_t x)noexcept {
  words_[index(m)][i]=x;
  switch(m){case Model::super:super_.set_inputs(words_[1],dirty_[1]);break;case Model::harmo:(void)harmo_.set_word(i,x);break;case Model::karplus:(void)karplus_.set_word(i,x);break;case Model::va:va_controls_.words[i]=x;break;case Model::waveshaper:(void)waveshaper_.try_set_control_word(i,x);break;case Model::bass:bass_.set_inputs(words_[6],dirty_[6]);break;default:break;}
 }
 void dirty(Model m,unsigned i,std::uint8_t x)noexcept {
  dirty_[index(m)][i]=x;
  switch(m){case Model::super:super_.set_inputs(words_[1],dirty_[1]);break;case Model::harmo:(void)harmo_.set_dirty(i,x);break;case Model::karplus:(void)karplus_.set_dirty(i,x);break;case Model::va:va_controls_.dirty[i]=x;break;case Model::waveshaper:(void)waveshaper_.try_set_control_dirty(i,x);break;case Model::bass:bass_.set_inputs(words_[6],dirty_[6]);break;default:break;}
 }
 void pitch(Model m,float hz,float note)noexcept {
  switch(m){case Model::basic:basic_.set_pitch(hz,note);break;case Model::super:super_.set_pitch(hz,note);break;case Model::harmo:harmo_.set_frequency(hz);break;case Model::karplus:karplus_.set_pitch(hz,note);break;case Model::va:va_controls_.parameters.note=note;break;case Model::waveshaper:waveshaper_.set_pitch(hz,note);break;case Model::bass:bass_.set_pitch(hz);break;}
 }
 void phase(Model m,float x)noexcept {
  switch(m){case Model::basic:basic_.set_phase_seed(x);break;case Model::super:super_.set_phase(x);break;case Model::harmo:harmo_.set_phase(x);break;case Model::karplus:karplus_.set_phase(x);break;case Model::va:break;case Model::waveshaper:waveshaper_.set_phase(x);break;case Model::bass:bass_.set_phase(x);break;}
 }
 void bind(Model m,std::uint32_t count)noexcept {switch(m){case Model::harmo:harmo_.bind_buffer(true,count,identity_);break;case Model::karplus:karplus_.bind(identity_,static_cast<std::int32_t>(count));break;case Model::waveshaper:waveshaper_.bind_buffer(identity_,static_cast<std::int32_t>(count));break;default:break;}}
 // These are native INTERFACE slots, deliberately not generic C++ method names.
 void initialize(Model m)noexcept {switch(m){case Model::basic:basic_.initialize();break;case Model::super:super_.repoint();break;case Model::harmo:harmo_.initialize();break;case Model::karplus:karplus_.initialize();break;case Model::va:va_.initialize();break;case Model::waveshaper:waveshaper_.initialize();break;case Model::bass:break;}}
 void reset(Model m)noexcept {switch(m){case Model::basic:basic_.reset();break;case Model::super:super_.initialize();break;case Model::harmo:harmo_.reset();break;case Model::karplus:karplus_.reset();break;case Model::va:break;case Model::waveshaper:waveshaper_.reset();break;case Model::bass:bass_.initialize();break;}}
 void trigger(Model m,std::uint8_t x)noexcept {switch(m){case Model::basic:basic_.trigger(x);break;case Model::super:super_.trigger(x);break;case Model::harmo:harmo_.trigger(x!=0);break;case Model::karplus:karplus_.trigger(x!=0);break;case Model::va:va_controls_.signal=x;break;case Model::waveshaper:waveshaper_.trigger(x!=0);break;case Model::bass:bass_.trigger(x);break;}}
 void prepare(Model m)noexcept {switch(m){case Model::basic:basic_.prepare_controls(words_[0],dirty_[0]);break;case Model::super:super_.prepare_controls(true);break;case Model::harmo:harmo_.prepare_current();break;case Model::karplus:karplus_.prepare_current(true);break;case Model::va:mfrk::update_virtual_analog_controls(va_controls_);va_controls_.dirty={};break;case Model::waveshaper:waveshaper_.prepare(true);break;case Model::bass:bass_.prepare_controls(true);break;}dirty_[index(m)]={};}
 // Valid Model values, control indices 0..2, source-defined controls/pitch,
 // and initialize-before-render are required. This is not a full voice graph.
 // Failure from checked component domains is explicit, never silent substitute audio.
 bool render(Model m,std::array<float,32>&out)noexcept;
};
// Source selection path for an already-initialized Basic slot with empty
// manager metadata. The outer dirty bytes are distinct from each model's dirty
// bytes; callers forward them through the native control path separately.
class SlotController {
 Side& side_;
 Model selected_=Model::basic;
 float phase_=0;
 std::array<std::uint8_t,3> dirty_{};
 std::array<std::uint16_t,3> words_{};
 std::array<std::uint16_t,9> pitch_words_{};
 std::array<std::uint8_t,9> pitch_dirty_{};
 slot_pitch::Pitch pitch_=slot_pitch::Pitch::admitted_zero_owner_arena();
public:
 explicit SlotController(Side& side) noexcept : side_(side) {}
 SlotController(const SlotController&)=delete;
 SlotController& operator=(const SlotController&)=delete;
 void phase(float value) noexcept { phase_=value; }
 Model selected() const noexcept { return selected_; }
 const auto& dirty() const noexcept { return dirty_; }
 void clear_dirty() noexcept { dirty_={}; }
 const auto& pitch_dirty() const noexcept {return pitch_dirty_;}
 bool set_model_control(unsigned index,std::uint16_t word,std::uint8_t dirty) noexcept;
 bool set_pitch_control(unsigned index,std::uint16_t word,std::uint8_t dirty) noexcept;
 // Original slot preparation: forward dirty model controls, compile pitch,
 // dispatch updated pitch and prepare the selected persistent model. Outer
 // model dirty bytes remain until clear_dirty(); pitch dirty bytes are consumed.
 bool prepare(pitch_update::Slot&,slot_pitch::Result&) noexcept;
 // False means outside this bank's model profile. An already-active model is a no-op.
 bool select(Model model) noexcept;
};
}
