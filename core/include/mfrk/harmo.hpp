#pragma once
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

namespace mfrk::harmo {
struct TestAccess;
class Engine;
class Tables {
    friend class Engine;
    friend struct TestAccess;
    std::array<std::int32_t,8> norm{};
    std::array<std::uint8_t,16544> auxiliary{};
    template<class T> T at(std::size_t offset) const noexcept {
        assert(offset+sizeof(T)<=auxiliary.size());
        T value;std::memcpy(&value,auxiliary.data()+offset,sizeof value);return value;
    }
    template<class T> void put(std::size_t offset,T value) noexcept {
        assert(offset+sizeof(T)<=auxiliary.size());
        std::memcpy(auxiliary.data()+offset,&value,sizeof value);
    }
public:
    // Mathematical initialization; construct outside the audio callback.
    // Nearest rounding, masked FP exceptions, gradual underflow (FTZ/DAZ off).
    Tables() noexcept;
};
struct Input {
    float frequency;
    std::array<std::uint16_t,3> words;
    std::array<std::uint8_t,3> dirty;
    bool trigger,reset_smoothers;
    float gain;
};
// Fixed32frames at96kHz. Tables must outlive the engine. Finite positive Hz,
// finite phase/gain, nearest rounding, masked FP exceptions and gradual
// underflow (FTZ/DAZ off). CVTT overflow follows original integer
// indefinite semantics; high frequencies are not clamped to an invented range.
class Engine {
    friend struct TestAccess;
    const Tables& tables;
    // Internal value storage only. Canonical native address words are inert
    // oracle labels, never process addresses or dereferenced pointers.
    std::array<std::uint8_t,424> state{};
    std::array<std::int16_t,2048> delay{};
    std::int16_t* external_delay=nullptr;
    auto delay_words() noexcept { return std::span<std::int16_t,2048>(external_delay?external_delay:delay.data(),2048); }
    auto delay_words() const noexcept { return std::span<const std::int16_t,2048>(external_delay?external_delay:delay.data(),2048); }
    template<class T=std::int32_t> T get(std::size_t offset) const noexcept {
        assert(offset+sizeof(T)<=state.size());
        T value;std::memcpy(&value,state.data()+offset,sizeof value);return value;
    }
    template<class T> void put(std::size_t offset,T value) noexcept {
        assert(offset+sizeof(T)<=state.size());
        std::memcpy(state.data()+offset,&value,sizeof value);
    }
    void word(std::size_t offset,std::int64_t value) noexcept;
    void integer(std::size_t offset,std::int64_t value) noexcept;
    Engine(const Tables&,bool delay_enabled,std::int16_t* external=nullptr,
           bool initialize_now=true) noexcept;
    void target(int base,float value) noexcept;
    float smoother(int base) noexcept;
    void controls() noexcept;
    int pitch() noexcept;
    std::array<std::int16_t,16> harmonic(int bank) noexcept;
    void delay_process(std::array<std::int16_t,16>&) noexcept;
public:
    // Explicit zero preconstructor arena and zero owned delay profile. Unwritten
    // bytes are supplied by this admission, not inferred native factory defaults.
    static Engine admitted_zero_arena(const Tables& tables,bool delay_enabled) noexcept {
        return Engine(tables,delay_enabled);
    }
    // Native owner-pool profile: raw construction, derived class/gain stores,
    // no bind or Init. Borrow the first2048 words of a persistent slot resource.
    // Memory must outlive this model; construction/Init do not clear it.
    // Copies of this borrowed profile share the same storage.
    static Engine admitted_zero_arena_uninitialized(const Tables& tables,
                                     std::span<std::int16_t,2048> memory) noexcept {
        return Engine(tables,false,memory.data(),false);
    }
    // Binding changes saved metadata; initialize commits it for rendering.
    // identity_label is inert provenance, never a host address. Actual storage
    // remains the fixed owned/borrowed buffer; binding does not migrate memory.
    void bind_buffer(bool enabled,std::uint32_t count,
                     std::uint64_t identity_label=0x400003000ull) noexcept;
    void initialize() noexcept;
    void trigger(bool value) noexcept;
    void reset() noexcept;
    bool set_word(unsigned index,std::uint16_t value) noexcept;
    bool set_dirty(unsigned index,std::uint8_t value) noexcept;
    // Native interface prepare: read stored words/dirty, prepare, then clear flags.
    // Set positive finite frequency before calling, as required by prepare().
    void prepare_current() noexcept;
    void set_frequency(float value) noexcept;
    void set_phase(float phase) noexcept;
    void prepare(const Input&) noexcept;
    // Establish positive finite frequency with prepare/set_frequency before
    // render_*; the explicitly admitted zero arena has core frequency zero.
    std::array<std::int16_t,32> render_q15() noexcept;
    std::array<float,32> render_decoded() noexcept;
    std::array<float,32> render_scaled() noexcept;
    std::array<float,32> process(const Input&) noexcept;
};
} // namespace mfrk::harmo
