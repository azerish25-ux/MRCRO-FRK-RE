#include "mfrk/deferred_vcf49.hpp"

#include <atomic>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <new>
#include <stdexcept>
#include <thread>

namespace {
thread_local bool audio_region = false;
thread_local bool fail_next_allocation = false;
std::atomic<std::size_t> audio_allocations{0};
struct AudioRegion {
    AudioRegion() { audio_region = true; }
    ~AudioRegion() { audio_region = false; }
};
struct Reference {
    std::ifstream file;
    explicit Reference(const char* path) : file(path,std::ios::binary) { if (!file) throw std::runtime_error("Missing lifecycle fixture"); }
    std::uint32_t word(unsigned bytes = 4) {
        std::uint32_t result = 0;
        for (unsigned i = 0; i < bytes; ++i) {
            const int c = file.get(); if (c < 0) throw std::runtime_error("Truncated lifecycle fixture");
            result |= static_cast<std::uint32_t>(c) << (i*8);
        }
        return result;
    }
    float scalar() { return std::bit_cast<float>(word()); }
};
std::uint32_t bits(float value) { return std::bit_cast<std::uint32_t>(value); }
}  // namespace

void* operator new(std::size_t count) {
    if (fail_next_allocation) { fail_next_allocation = false; throw std::bad_alloc(); }
    if (audio_region) audio_allocations.fetch_add(1,std::memory_order_relaxed);
    if (void* result = std::malloc(count == 0 ? 1 : count)) return result;
    throw std::bad_alloc();
}
void* operator new[](std::size_t count) { return ::operator new(count); }
void operator delete(void* value) noexcept { std::free(value); }
void operator delete[](void* value) noexcept { std::free(value); }
void operator delete(void* value,std::size_t) noexcept { std::free(value); }
void operator delete[](void* value,std::size_t) noexcept { std::free(value); }

int main(int argc,char** argv) try {
    if (argc != 2) throw std::runtime_error("Expected lifecycle fixture path");
    Reference reference(argv[1]);
    if (reference.word() != 0x4c46564d || reference.word() != 2) throw std::runtime_error("Invalid lifecycle fixture");
    const auto commands = reference.word();
    if (commands > 10000 || commands == 0) throw std::runtime_error("Invalid command count");
    std::unique_ptr<mfrk::DeferredVcf49> filter;
    std::array<float,32> audio{},modulation{};
    std::size_t failures = 0, samples = 0, snapshots = 0, suspended = 0, parameter_snapshots = 0;
    for (std::uint32_t command = 0; command < commands; ++command) {
        const auto op = reference.word(1);
        switch (op) {
        case 0: {
            const auto seed = reference.word(); const bool adaptive = reference.word() != 0;
            filter = std::make_unique<mfrk::DeferredVcf49>(seed,adaptive); break;
        }
        case 1: case 2: {
            const auto parameter = static_cast<mfrk::Vcf49Parameter>(reference.word()); const float value = reference.scalar();
            AudioRegion guard;
            if (op == 1) filter->set_normalized(parameter,value); else filter->set_physical(parameter,value);
            break;
        }
        case 3: filter->initialize(reference.scalar()); break;
        case 4: {
            const bool direct = reference.word() != 0;
            if (direct) filter->prepare(true);
            else { AudioRegion guard; filter->prepare(false); }
            break;
        }
        case 5: {
            for (auto& value : audio) value = reference.scalar();
            for (auto& value : modulation) value = reference.scalar();
            std::array<std::uint32_t,32> expected;
            for (auto& value : expected) value = reference.word();
            std::array<float,32> output{};
            bool processed;
            { AudioRegion guard; processed = filter->render(audio,modulation,output); }
            if (!processed) ++failures;
            for (std::size_t i = 0; i < output.size(); ++i) {
                if (bits(output[i]) != expected[i]) {
                    if (failures < 8) std::printf("Command %u sample %zu: %08x != %08x\n",command,i,bits(output[i]),expected[i]);
                    ++failures;
                }
                ++samples;
            }
            break;
        }
        case 6: { AudioRegion guard; filter->end_block(); break; }
        case 7: {
            bool processed = false;
            std::exception_ptr error;
            std::thread worker([&] { try { processed = filter->worker_step(); } catch (...) { error = std::current_exception(); } });
            worker.join();
            if (error) std::rethrow_exception(error);
            if (!processed) ++failures;
            break;
        }
        case 8: break;
        case 9: {
            const auto before = filter->state().noise_seed;
            std::array<float,32> output;
            output.fill(std::bit_cast<float>(std::uint32_t{0x5a5a5a5a}));
            bool processed;
            { AudioRegion guard; processed = filter->render(audio,modulation,output); }
            if (processed || filter->state().noise_seed != before) ++failures;
            for (float value : output) if (bits(value) != 0x5a5a5a5a) ++failures;
            ++suspended;
            break;
        }
        case 10: {
            const auto count = reference.word();
            if (count > 5) throw std::runtime_error("Bad pending count");
            const auto state = filter->state();
            if (state.pending_count != count) ++failures;
            for (std::uint32_t i = 0; i < count; ++i) {
                const auto parameter = reference.word(), value = reference.word();
                if (static_cast<std::uint32_t>(state.pending[i].parameter) != parameter || state.pending[i].normalized_bits != value) ++failures;
            }
            break;
        }
        case 11: {
            const auto parameter = static_cast<mfrk::Vcf49Parameter>(reference.word()); const auto expected = reference.word();
            if (bits(filter->normalized(parameter)) != expected) ++failures;
            break;
        }
        default: throw std::runtime_error("Bad lifecycle opcode");
        }
        const auto mask = reference.word();
        std::array<std::uint32_t,12> expected;
        for (auto& value : expected) value = reference.word();
        if (mask >> 12) throw std::runtime_error("Bad lifecycle mask");
        if (mask) {
            const auto state = filter->state();
            const std::array<std::uint32_t,12> actual{state.ready,bits(state.fade.current),bits(state.fade.target),state.resume_fade,
                state.factor,state.kernel,state.latency,state.latency_changed,state.first,state.force,state.noise_seed,
                static_cast<std::uint32_t>(state.work)};
            for (std::size_t i = 0; i < actual.size(); ++i) if ((mask & (1u<<i)) && actual[i] != expected[i]) {
                if (failures < 8) std::printf("Command %u state %zu: %08x != %08x\n",command,i,actual[i],expected[i]);
                ++failures;
            }
            ++snapshots;
        }
        const auto count = reference.word();
        if (count > 7) throw std::runtime_error("Bad parameter snapshot count");
        for (std::uint32_t i = 0; i < count; ++i) {
            const auto id = reference.word();
            if (id > 6) throw std::runtime_error("Bad parameter snapshot ID");
            const auto value = filter->parameter_snapshot(id);
            const std::array<std::uint32_t,5> actual{value.normalized_bits,value.physical_bits,value.status,value.dirty,value.aggregate_dirty};
            for (std::size_t j = 0; j < actual.size(); ++j) if (actual[j] != reference.word()) {
                if (failures < 8) std::printf("Command %u parameter %u field %zu mismatch\n",command,id,j);
                ++failures;
            }
            ++parameter_snapshots;
        }
    }
    if (reference.file.peek() != std::char_traits<char>::eof()) throw std::runtime_error("Trailing lifecycle data");
    // Portable ownership/error contract, separate from the original numeric gold.
    // A failed private generation must leave the old state/request available.
    std::size_t allocation_recoveries = 0;
    if (failures == 0) {
        mfrk::DeferredVcf49 recovery(1,false);
        recovery.initialize(96000.0f);
        recovery.set_physical(mfrk::Vcf49Parameter::factor_index,1.0f);
        { AudioRegion guard; recovery.prepare(false); }
        std::array<float,32> input{}, output{};
        { AudioRegion guard; recovery.render(input,input,output); recovery.end_block(); }
        const auto before = recovery.state();
        bool rejected = false;
        std::exception_ptr error;
        std::thread failing_worker([&] {
            fail_next_allocation = true;
            try { recovery.worker_step(); }
            catch (const std::bad_alloc&) { rejected = true; }
            catch (...) { error = std::current_exception(); }
        });
        failing_worker.join();
        if (error) std::rethrow_exception(error);
        const auto after = recovery.state();
        if (!rejected || after.ready || after.work != mfrk::FilterWorkPhase::requested ||
            after.factor != before.factor || after.noise_seed != before.noise_seed) ++failures;
        bool retried = false;
        std::thread retry([&] { try { retried = recovery.worker_step(); } catch (...) { error = std::current_exception(); } });
        retry.join();
        if (error) std::rethrow_exception(error);
        { AudioRegion guard; recovery.prepare(false); }
        if (!retried || !recovery.state().ready || recovery.state().factor != 4) ++failures;
        else ++allocation_recoveries;
    }
    failures += audio_allocations.load();
    std::printf("Original deferred filter: %u commands, %zu samples, %zu snapshots, %zu parameter snapshots, %zu suspended checks, %zu audio allocations, %zu mismatches\n",
                commands,samples,snapshots,parameter_snapshots,suspended,audio_allocations.load(),failures);
    std::printf("Portable failed-allocation recoveries: %zu\n",allocation_recoveries);
    return failures == 0 ? 0 : 1;
} catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what()); return 2; }
