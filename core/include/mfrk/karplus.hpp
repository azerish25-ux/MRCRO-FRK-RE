#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
namespace mfrk::karplus {
struct SharedRandom {std::uint32_t value;}; // Actual module-global state, not pervoice.
struct Memory {std::array<std::int16_t,8192> samples;}; // Explicit caller contents.
struct TestAccess;
class Engine;
class Tables {
 friend class Engine;
 friend struct TestAccess;
 std::array<std::int32_t,552> coefficients{};
public:
 Tables() noexcept;
};
enum class Status {ok,not_initialized,table_domain,math_domain,invalid_input,unbound};
struct Input {float frequency_hz,note;std::array<std::uint16_t,3> words;std::array<std::uint8_t,3> dirty;int trigger_event=0;bool interface_prepare=false;float level=std::bit_cast<float>(0x40086f69u);}; // trigger_event0none,1off,2on.
class Engine {
 friend struct TestAccess;
 const Tables& tables_;SharedRandom& random_;Memory& memory_;
 std::array<std::uint8_t,288> state_{};std::array<std::int16_t,32> raw_{},excitation_{};bool initialized_=false;
 struct ZeroTag{};
 Engine(const Tables&t,SharedRandom&r,Memory&m,ZeroTag) noexcept;
 template<class T=std::int32_t>T get(unsigned o)const noexcept{T v;std::memcpy(&v,state_.data()+o,sizeof v);return v;}
 template<class T>void put(unsigned o,T v)noexcept{std::memcpy(state_.data()+o,&v,sizeof v);}
 void integer(unsigned o,std::int64_t x)noexcept;
 void short_word(unsigned o,std::int64_t x)noexcept;
 void construct()noexcept;
 Status coefficient_pair(float cutoff,int bank,std::int32_t&a,std::int32_t&b)noexcept;
 Status excite()noexcept;
 Status string()noexcept;
 Status core()noexcept;
public:
 // Persistent model: Tables, SharedRandom and Memory must outlive this engine.
 Engine(const Engine&)=delete;
 Engine& operator=(const Engine&)=delete;
 Engine(Engine&&)=delete;
 Engine& operator=(Engine&&)=delete;
 // Explicitly supplied zero preconstructor arena; does not clear caller Memory.
 static Engine admitted_zero_arena(const Tables&t,SharedRandom&r,Memory&m)noexcept;
 // Source address fields are inert identity labels, never host pointers.
 // Actual storage is the fixed Memory supplied at construction. A pending bind
 // commits on initialize(); selection does not clear Memory or old histories.
 void bind(std::uint64_t token,std::int32_t count)noexcept;
 void initialize()noexcept;
 // Raw model reset and phase setters are source no-ops; whole-slot reset is separate.
 void reset()noexcept;
 void set_phase(float)noexcept;
 bool set_word(unsigned index,std::uint16_t value)noexcept;
 bool set_dirty(unsigned index,std::uint8_t value)noexcept;
 void set_flag(std::uint8_t value)noexcept;
 std::uint8_t flag()const noexcept;
 void fetch_words(const std::array<std::uint16_t,3>& words)noexcept;
 void initialize_from_records(const std::array<std::uint16_t,3>& words)noexcept;
 void prepare_current(bool interface=false)noexcept;
 void set_level(float value)noexcept;
 void set_pitch(float hz,float note)noexcept;
 void trigger(bool on)noexcept;
 void prepare(const Input&in)noexcept;
 // Exactly 32 frames at 96 kHz. Source nearest rounding and gradual underflow.
 // Domain/binding failures leave state, Memory, RNG and caller output unchanged.
 Status try_render(std::array<float,32>&output,bool scaled=true)noexcept;
 Status try_process(const Input&in,std::array<float,32>&output)noexcept;
};
}
