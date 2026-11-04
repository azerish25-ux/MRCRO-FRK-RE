#pragma once
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>

namespace mfrk::waveshaper {
struct TestAccess;
class Engine;
// Immutable mathematical lookup tables. Construct outside the audio callback.
// Nearest-even rounding, masked FP exceptions and gradual underflow are required.
class Tables {
    friend class Engine;
    friend struct TestAccess;
    std::array<float,256> coarse{},fine{};
    std::array<float,516> fold{};
    std::array<std::array<std::int16_t,257>,5> shape{};
    float frequency(float note) const noexcept;
public:
    Tables() noexcept;
};
enum class RenderStatus { ok, not_initialized, shape_domain, fold_domain };
struct Input {
    float pitch_argument; // Original second float/XMM2; first frequency is ignored.
    std::array<std::uint16_t,3> words;
    std::array<std::uint8_t,3> dirty;
    bool gate=false,initialize=false,reset=false,interface_prepare=false;
    float output_level=std::bit_cast<float>(std::uint32_t{0x3f79999a});
};
// Fixed32-frame Type5 engine at96kHz. Tables must outlive the engine.
// This is a source-bound component, not a host parser or complete voice graph.
class Engine {
    friend struct TestAccess;
    const Tables& tables_;
    bool initialized_=false; // Admission metadata, not an original state field.
    // Canonical native address words are inert oracle labels, never dereferenced.
    std::array<std::uint8_t,216> state_{};
    std::array<float,16> slope_{},folded_{};
    struct ZeroArenaTag {};
    Engine(const Tables&,ZeroArenaTag) noexcept;
    template<class T=float> T get(unsigned offset) const noexcept {
        T value;std::memcpy(&value,state_.data()+offset,sizeof value);return value;
    }
    template<class T> void put(unsigned offset,T value) noexcept {
        std::memcpy(state_.data()+offset,&value,sizeof value);
    }
    static std::int32_t cvtt32(float) noexcept;
    void construct() noexcept;
    static float integrated(float u,bool negative=false) noexcept;
    void oscillator(float frequency,float pw) noexcept;
    RenderStatus core() noexcept;
public:
    // Explicit zero preconstructor arena.164bytes are unwritten by the native
    // constructor; zero is supplied by this admission, not inferred heap policy.
    // initialize() is required before rendering. It preserves outer history.
    [[nodiscard]] static Engine admitted_zero_arena(const Tables& tables) noexcept;
    void initialize() noexcept;
    void reset() noexcept; // Original routed reset is a no-op.
    void set_phase(float) noexcept; // Original routed phase setter is a no-op.
    void set_pitch(float unused_frequency,float pitch_argument) noexcept;
    void trigger(bool gate) noexcept;
    // Binding metadata only; identified Init/render never dereference this token.
    void bind_buffer(std::uint64_t opaque_token,std::int32_t count) noexcept;
    void refresh_controls(std::array<std::uint16_t,3>) noexcept;
    void fetch_and_initialize(std::array<std::uint16_t,3>) noexcept;
    [[nodiscard]] std::uint8_t interface_flag() const noexcept;
    void set_controls(std::array<std::uint16_t,3>,std::array<std::uint8_t,3>) noexcept;
    void prepare(bool interface_clear_dirty=false) noexcept;
    // All index checks remain active in Release. Failure leaves output, source
    // state and traces unchanged. This is NOT native-equivalent fault audio.
    [[nodiscard]] RenderStatus try_render_raw(std::array<float,32>&) noexcept;
    [[nodiscard]] RenderStatus try_render_scaled(std::array<float,32>&) noexcept;
    // Also rolls back input stores/Init/prepare/admission on failure.
    [[nodiscard]] RenderStatus try_process(const Input&,std::array<float,32>&) noexcept;
    [[nodiscard]] bool try_set_control_word(unsigned,std::uint16_t) noexcept;
    [[nodiscard]] bool try_set_control_dirty(unsigned,std::uint8_t) noexcept;
};
} // namespace mfrk::waveshaper
