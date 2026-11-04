#include "mfrk/fx_input.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
using namespace mfrk::fx_input;
struct Reader {
  std::ifstream file;
  explicit Reader(const char* path) : file(path, std::ios::binary) {
    if (!file) throw std::runtime_error("fixture open failed");
  }
  std::uint32_t integer(unsigned count) {
    std::uint32_t value = 0;
    for (unsigned i = 0; i < count; ++i) {
      const int byte = file.get();
      if (byte < 0) throw std::runtime_error("truncated fixture");
      value |= std::uint32_t(byte) << (8 * i);
    }
    return value;
  }
  float number() { return std::bit_cast<float>(integer(4)); }
  std::int16_t short_integer() {
    return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(integer(2)));
  }
  GainHistory gains() {
    GainHistory history;
    for (auto& pair : history) { pair.left = number(); pair.right = number(); }
    return history;
  }
};
std::uint32_t bits(float value) { return std::bit_cast<std::uint32_t>(value); }
int main(int argc, char** argv) try {
  if (argc != 2) throw std::runtime_error("usage: fx_input_test original-fx-input-fixtures.bin");
  Reader r(argv[1]);
  std::string magic;
  for (unsigned i = 0; i < 8; ++i) magic += char(r.integer(1));
  if (magic != "FXVCA001") throw std::runtime_error("fixture magic");
  std::uint64_t sample_checks = 0, sample_bad = 0, history_checks = 0, history_bad = 0;
  std::uint64_t source_checks = 0, source_bad = 0, vca_checks = 0, vca_bad = 0;
  std::uint64_t norm_checks = 0, norm_bad = 0;
  std::optional<InputGainPan> stage;
  const auto prefix_cases = r.integer(4);
  for (unsigned c = 0; c < prefix_cases; ++c) {
    const bool alternate = r.integer(1) != 0;
    const bool disabled = r.integer(1) != 0;
    const float width = r.number();
    const auto block = r.integer(4);
    MonoInputs inputs;
    Targets envelopes, pans;
    for (auto& lane : inputs) for (auto& value : lane) value = r.number();
    for (auto& value : envelopes) value = r.number();
    for (auto& value : pans) value = r.number();
    const auto previous = r.gains();
    const auto next = r.gains();
    if (block == 0) stage.emplace(previous);
    if (!stage) throw std::runtime_error("missing explicit starting history");
    for (unsigned lane = 0; lane < lane_count; ++lane) {
      history_checks += 2;
      history_bad += bits(stage->history()[lane].left) != bits(previous[lane].left);
      history_bad += bits(stage->history()[lane].right) != bits(previous[lane].right);
    }
    const auto actual = stage->process(inputs, envelopes, pans, {width, disabled, alternate});
    for (const auto& lane : actual) {
      for (float value : lane.left) { ++sample_checks; sample_bad += bits(value) != r.integer(4); }
      for (float value : lane.right) { ++sample_checks; sample_bad += bits(value) != r.integer(4); }
    }
    for (unsigned lane = 0; lane < lane_count; ++lane) {
      history_checks += 2;
      history_bad += bits(stage->history()[lane].left) != bits(next[lane].left);
      history_bad += bits(stage->history()[lane].right) != bits(next[lane].right);
    }
  }
  const auto controller_cases = r.integer(4);
  for (unsigned c = 0; c < controller_cases; ++c) {
    const auto adsr = r.short_integer();
    const auto velocity = static_cast<std::uint8_t>(r.integer(1));
    const auto env_sensitivity = r.short_integer();
    const auto vca_sensitivity = r.short_integer();
    const auto effective = r.short_integer();
    const auto original_para_flag = r.integer(1);
    (void)original_para_flag;  // Scope: arithmetic only; routing was separately probed.
    const auto source = envelope_source_q15(adsr, velocity, env_sensitivity);
    const auto gain = vca_gain_q14(source, velocity, vca_sensitivity, effective);
    for (unsigned lane = 0; lane < 12; ++lane) { ++source_checks; source_bad += source != r.short_integer(); }
    for (unsigned lane = 0; lane < 12; ++lane) { ++vca_checks; vca_bad += gain != r.short_integer(); }
  }
  const auto normalization_cases = r.integer(4);
  for (unsigned c = 0; c < normalization_cases; ++c) {
    const auto input = r.short_integer();
    const auto word = r.integer(2);
    const auto expected_bits = r.integer(4);
    norm_checks += 2;
    norm_bad += canonical_word_from_q14(input) != word;
    norm_bad += bits(normalized_gain_from_q14(input)) != expected_bits;
  }
  if (r.file.peek() != EOF) throw std::runtime_error("trailing fixture data");
  std::cout << "{\"prefix_cases\":" << prefix_cases
            << ",\"stereo_sample_checks\":" << sample_checks << ",\"sample_mismatches\":" << sample_bad
            << ",\"history_checks\":" << history_checks << ",\"history_mismatches\":" << history_bad
            << ",\"controller_cases\":" << controller_cases
            << ",\"source_word_checks\":" << source_checks << ",\"source_word_mismatches\":" << source_bad
            << ",\"vca_word_checks\":" << vca_checks << ",\"vca_word_mismatches\":" << vca_bad
            << ",\"normalization_checks\":" << norm_checks << ",\"normalization_mismatches\":" << norm_bad << "}\n";
  return sample_bad || history_bad || source_bad || vca_bad || norm_bad ? 1 : 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n';
  return 2;
}
