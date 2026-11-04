#include "mfrk/fx_mix.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace mfrk::fx_input;
namespace {
class Reader {
public:
  explicit Reader(const char* path) : stream_(path, std::ios::binary) {
    if (!stream_) throw std::runtime_error("fixture open failed");
  }
  std::uint32_t integer(unsigned bytes) {
    std::uint32_t value = 0;
    for (unsigned i = 0; i < bytes; ++i) {
      const int byte = stream_.get();
      if (byte < 0) throw std::runtime_error("truncated fixture");
      value |= std::uint32_t(byte) << (8 * i);
    }
    return value;
  }
  float number() { return std::bit_cast<float>(integer(4)); }
  void require_end() {
    if (stream_.get() != std::char_traits<char>::eof()) {
      throw std::runtime_error("trailing fixture bytes");
    }
  }
private:
  std::ifstream stream_;
};
std::uint32_t bits(float x) { return std::bit_cast<std::uint32_t>(x); }
}

int main(int argc, char** argv) try {
  if (argc != 2) throw std::runtime_error("usage: fx_mix_test original-fx-mix-fixtures.bin");
  Reader reader(argv[1]);
  std::string magic;
  for (unsigned i = 0; i < 8; ++i) magic += char(reader.integer(1));
  if (magic != "FXMIX001") throw std::runtime_error("fixture magic");
  std::uint64_t sum_checks = 0, sum_bad = 0, cold_checks = 0, cold_bad = 0;
  const auto cases = reader.integer(4);
  for (std::uint32_t c = 0; c < cases; ++c) {
    const bool left_present = reader.integer(1) != 0;
    const bool right_present = reader.integer(1) != 0;
    StereoLanes lanes;
    for (auto& lane : lanes) {
      for (auto& sample : lane.left) sample = reader.number();
      for (auto& sample : lane.right) sample = reader.number();
    }
    StereoSamples external;
    for (auto& sample : external.left) sample = reader.number();
    for (auto& sample : external.right) sample = reader.number();
    const auto actual = sum_lanes(lanes, {left_present ? &external.left : nullptr,
                                         right_present ? &external.right : nullptr});
    for (const auto* channel : {&actual.left, &actual.right}) {
      for (const auto sample : *channel) {
        ++sum_checks;
        sum_bad += bits(sample) != reader.integer(4);
      }
    }
  }
  const auto cold_cases = reader.integer(4);
  for (std::uint32_t c = 0; c < cold_cases; ++c) {
    const auto actual = cold_constructor_defaults();
    for (const auto pair : actual.history) {
      for (const auto sample : {pair.left, pair.right}) {
        ++cold_checks;
        cold_bad += bits(sample) != reader.integer(4);
      }
    }
    cold_checks += 3;
    cold_bad += bits(actual.pan.width) != reader.integer(4);
    cold_bad += actual.pan.disable_spread != (reader.integer(1) != 0);
    cold_bad += actual.pan.alternate_pattern != (reader.integer(2) != 0);
  }
  reader.require_end();
  std::cout << "{\"sum_cases\":" << cases << ",\"sum_checks\":" << sum_checks
            << ",\"sum_mismatches\":" << sum_bad << ",\"cold_checks\":" << cold_checks
            << ",\"cold_mismatches\":" << cold_bad << "}\n";
  return sum_bad || cold_bad ? 1 : 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n';
  return 2;
}
