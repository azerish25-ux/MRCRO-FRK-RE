#include "mfrk/adsr.hpp"
#include "adsr_test_access.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
using namespace mfrk;
#include "adsr_field_adapters.inc"
struct Reader {
  std::ifstream file;
  explicit Reader(const char* path):file(path,std::ios::binary) {
    if (!file) throw std::runtime_error("fixture open failed");
  }
  std::uint64_t integer(unsigned bytes) {
    std::uint64_t v=0;
    for (unsigned i=0;i<bytes;++i) {
      int c=file.get();
      if (c<0)throw std::runtime_error("truncated fixture");
      v|=std::uint64_t(c)<<(8*i);
    }
    return v;
  }
  // One state vector for EVERY fields() call, including unasserted expected
  // arrays and explicit op6 fixture-state loads. Never reset at commands/masks.
  std::array<std::int64_t, 33> previous{};

  std::uint64_t varint() {
    std::uint64_t value = 0;
    for (unsigned index = 0; index < 10; ++index) {
      const auto byte = integer(1);
      const auto payload = byte & 127;
      if (index == 9 && payload > 1) {
        throw std::runtime_error("varint exceeds uint64");
      }
      value |= payload << (index * 7);
      if ((byte & 128) == 0) return value;
    }
    throw std::runtime_error("varint exceeds ten bytes");
  }

  std::array<std::int64_t, 33> fields() {
    const auto bitmap = varint();
    if (bitmap >> 33) throw std::runtime_error("field bitmap exceeds 33 bits");
    for (unsigned index = 0; index < 33; ++index) {
      if ((bitmap & (std::uint64_t{1} << index)) != 0) {
        const auto encoded = varint();
        const auto signed_bits = (encoded >> 1) ^ (std::uint64_t{0} - (encoded & 1));
        previous[index] = std::bit_cast<std::int64_t>(signed_bits);
      }
    }
    return previous;
  }
};
int main(int argc,char** argv) try {
  if (argc!=2)throw std::runtime_error("usage: adsr_compact_test compact-fixtures.bin");
  Reader r(argv[1]);
  std::string magic;
  for (int i=0;i<8;++i)magic+=char(r.integer(1));
  if (magic!="ADSRFX02")throw std::runtime_error("fixture magic");
  std::uint64_t table_mismatch=0,state_mismatch=0,field_mismatch=0,records=0,api_mismatch=0;
  std::vector<std::string> first;
  Adsr env;
  for (unsigned variant=0;variant<2;++variant) {
    auto data=Adsr::curve(variant);
    for (unsigned off=8;off<0x710;++off) {
      auto expected=r.integer(1);
      if (data[off]!=expected)++table_mismatch;
    }
  }

  // Portable admission rule: native unprepared fields have no established value.
  if (env.prepared()||env.gate(true)||env.step())++api_mismatch;
  if (env.snapshot().phase!=AdsrPhase::idle||env.snapshot().output!=0||env.snapshot().accumulator!=0)++api_mismatch;
  const auto commands=r.integer(4);
  for (std::uint64_t n=0;n<commands;++n) {
    auto op=r.integer(1);
    switch (op) {
      case 0:env=Adsr{};
      break;
      case 1: {
        AdsrControls words{};
        for (auto& w:words)w=static_cast<std::uint16_t>(r.integer(2));
        auto mask=static_cast<std::uint16_t>(r.integer(2));
        auto preserved=words;
        env.prepare(words,mask);
        if (words!=preserved||env.controls()!=words)++api_mismatch;
        break;
      }
      case 2:if (!env.gate(r.integer(1)!=0))++api_mismatch;
      break;
      case 3:if (!env.step())++api_mismatch;
      break;
      case 4:env.set_hold_flag(r.integer(1)!=0);
      break;
      case 5:env.reset();
      break;
      case 6:AdsrTestAccess::load_explicit_fixture(env,state_from(r.fields()));
      break;
      default:throw std::runtime_error("invalid fixture opcode");
    }
    auto mask=r.integer(8);
    auto expected=r.fields();
    if (mask) {
      ++records;
      auto got=flatten(env.snapshot());
      bool wrong=false;
      for (unsigned i=0;i<33;++i)if ((mask&(std::uint64_t {
        1
      }
      <<i))&&got[i]!=expected[i]) {
        wrong=true;
        ++field_mismatch;
        if (first.size()<8)first.push_back("command="+std::to_string(n)+" field="+std::to_string(i)+" got="+std::to_string(got[i])+" expected="+std::to_string(expected[i]));
      }
      if (wrong)++state_mismatch;
    }
  }
  if (r.file.peek()!=EOF)throw std::runtime_error("trailing fixture data");
  std::cout<<"{\"commands\":"<<commands<<",\"state_records\":"<<records<<",\"compared_table_bytes\":3600,\"table_byte_mismatches\":"<<table_mismatch<<",\"state_record_mismatches\":"<<state_mismatch<<",\"state_field_mismatches\":"<<field_mismatch<<",\"api_mismatches\":"<<api_mismatch<<",\"first_differences\":[";
  for (std::size_t i=0;i<first.size();++i)std::cout<<(i?",":"")<<"\""<<first[i]<<"\"";
  std::cout<<"]}\n";
  return table_mismatch||state_mismatch||api_mismatch?1:0;
} catch (const std::exception& e) {
  std::cerr<<e.what()<<'\n';
  return 2;
}
