#include "mfrk/superwave.hpp"

#include <bit>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace {
static_assert(std::endian::native == std::endian::little);
using Bytes = std::array<std::uint8_t,344>;
std::array<std::uint64_t,128> pointers;

struct Reader {
    std::vector<std::uint8_t> bytes;
    std::size_t position = 0;
    template<class T> void read(T& value) {
        if (sizeof(T) > bytes.size()-position) throw std::runtime_error("Truncated SuperWave fixture");
        std::memcpy(&value,bytes.data()+position,sizeof(T)); position += sizeof(T);
    }
    std::vector<std::uint8_t> take(std::size_t count) {
        if (count > bytes.size()-position) throw std::runtime_error("Truncated SuperWave expected data");
        std::vector<std::uint8_t> result(bytes.begin()+position,bytes.begin()+position+count);
        position += count; return result;
    }
};

std::uint32_t word(const std::vector<std::uint8_t>& bytes,std::size_t offset) {
    std::uint32_t value = 0;
    for (unsigned i=0;i<4;++i) value |= std::uint32_t{bytes[offset+i]} << (i*8);
    return value;
}
Reader fixture(const char* path) {
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if (!file) throw std::runtime_error("Missing packed SuperWave fixture");
    const auto length = file.tellg();
    if (length < 16 || length > 8*1024*1024) throw std::runtime_error("Invalid packed fixture size");
    std::vector<std::uint8_t> packed(static_cast<std::size_t>(length));
    file.seekg(0); if (!file.read(reinterpret_cast<char*>(packed.data()),length)) throw std::runtime_error("Cannot read packed fixture");
    if (std::memcmp(packed.data(),"SWZP0001",8)!=0) throw std::runtime_error("Invalid SuperWave packet header");
    const auto size = word(packed,8), expected_crc = word(packed,12);
    if (size == 0 || size > 16*1024*1024) throw std::runtime_error("Invalid decoded fixture size");
    Reader reader; reader.bytes.reserve(size);
    for (std::size_t position=16;position<packed.size();) {
        const auto tag = packed[position++]; const auto count = std::size_t{(tag&127u)+1};
        if (count > size-reader.bytes.size()) throw std::runtime_error("SuperWave decoded overflow");
        if (tag&128u) reader.bytes.insert(reader.bytes.end(),count,0);
        else {
            if (count > packed.size()-position) throw std::runtime_error("Truncated SuperWave literal");
            reader.bytes.insert(reader.bytes.end(),packed.begin()+position,packed.begin()+position+count); position += count;
        }
    }
    if (reader.bytes.size()!=size) throw std::runtime_error("Wrong SuperWave decoded size");
    std::uint32_t crc = 0xffffffffu;
    for (auto byte : reader.bytes) {
        crc ^= byte;
        for (unsigned i=0;i<8;++i) crc = (crc>>1)^((0u-(crc&1u))&0xedb88320u);
    }
    if ((crc^0xffffffffu)!=expected_crc) throw std::runtime_error("SuperWave canonical CRC mismatch");
    return reader;
}

template<class T> T get(const Bytes& bytes,std::size_t offset) {
    if (offset>bytes.size() || sizeof(T)>bytes.size()-offset) throw std::runtime_error("Invalid state read");
    T value; std::memcpy(&value,bytes.data()+offset,sizeof(value)); return value;
}
template<class T> void put(Bytes& bytes,std::size_t offset,T value) {
    if (offset>bytes.size() || sizeof(T)>bytes.size()-offset) throw std::runtime_error("Invalid state write");
    std::memcpy(bytes.data()+offset,&value,sizeof(value));
}
mfrk::SuperWaveTableSelection select(std::uint64_t pointer) {
    for (unsigned i=0;i<128;++i) if (pointers[i]==pointer) return {i/32,i%32};
    throw std::runtime_error("Unknown original table pointer");
}
mfrk::SuperWaveState decode(const Bytes& bytes) {
    auto s=mfrk::SuperWaveState::admitted({get<std::uint16_t>(bytes,8),get<std::uint16_t>(bytes,10),get<std::uint16_t>(bytes,12)},
                                        {bytes[14],bytes[15],bytes[16]});
    for (unsigned i=0;i<7;++i) {
        s.tables[i]=select(get<std::uint64_t>(bytes,0x30+i*8));
        s.bands[i]=get<std::uint32_t>(bytes,0x68+i*4); s.families[i]=get<std::uint32_t>(bytes,0x84+i*4);
        s.phase[i]=get<std::uint32_t>(bytes,0xa0+i*4); s.increment[i]=get<std::uint32_t>(bytes,0xbc+i*4);
        s.seed[i]=get<std::uint32_t>(bytes,0xd8+i*4); s.weight[i]=get<std::int16_t>(bytes,0x102+i*2);
    }
    s.wave_quartile=get<std::uint16_t>(bytes,0xf8); s.blend_fixed=get<std::int16_t>(bytes,0x100);
    s.detune=get<float>(bytes,0xfc); s.frequency=get<float>(bytes,0x114); s.table_key=get<float>(bytes,0x118);
    s.blend=get<float>(bytes,0x11c); s.target=get<float>(bytes,0x128); s.current=get<float>(bytes,0x12c);
    s.coefficient=get<float>(bytes,0x138); s.threshold=get<float>(bytes,0x13c);
    s.coefficients_initialized=true; s.force=bytes[0x140]; s.trigger_suppressed=bytes[0x141]; s.level=get<float>(bytes,0x28);
    return s;
}
Bytes encode(const mfrk::SuperWaveState& s,Bytes bytes) {
    for (unsigned i=0;i<3;++i) { put(bytes,8+i*2,s.words[i]); bytes[14+i]=s.dirty[i]; }
    for (unsigned i=0;i<7;++i) {
        const auto choice=s.tables[i]; if (choice.family>3 || choice.band>31) throw std::runtime_error("Invalid selected table");
        put(bytes,0x30+i*8,pointers[choice.family*32+choice.band]);
        put(bytes,0x68+i*4,s.bands[i]); put(bytes,0x84+i*4,s.families[i]); put(bytes,0xa0+i*4,s.phase[i]);
        put(bytes,0xbc+i*4,s.increment[i]); put(bytes,0xd8+i*4,s.seed[i]);
        if (s.coefficients_initialized) put(bytes,0x102+i*2,s.weight[i]);
    }
    put(bytes,0xf8,s.wave_quartile); put(bytes,0x100,s.blend_fixed); put(bytes,0xfc,s.detune);
    put(bytes,0x114,s.frequency); put(bytes,0x118,s.table_key); put(bytes,0x11c,s.blend);
    put(bytes,0x128,s.target); put(bytes,0x12c,s.current); put(bytes,0x138,s.coefficient); put(bytes,0x13c,s.threshold);
    bytes[0x140]=s.force; bytes[0x141]=s.trigger_suppressed; put(bytes,0x28,s.level); return bytes;
}
}  // namespace

int main(int argc,char** argv) try {
    if (argc!=2) throw std::runtime_error("Expected SuperWave fixture path");
    auto reader=fixture(argv[1]); std::array<char,8> magic; reader.read(magic);
    if (magic!=std::array<char,8>{'S','W','F','X','0','0','0','1'}) throw std::runtime_error("Invalid canonical header");
    reader.read(pointers); std::uint32_t count; reader.read(count);
    if (count==0 || count>100000) throw std::runtime_error("Invalid SuperWave record count");
    const mfrk::BasicWaveTables tables;
    std::array<unsigned,17> cases{},failures{}; unsigned failed=0;
    for (unsigned n=0;n<count;++n) {
        std::uint32_t kind,flag,length; float hz,key;
        reader.read(kind); reader.read(hz); reader.read(key); reader.read(flag); reader.read(length);
        if (kind>=cases.size()) throw std::runtime_error("Unknown SuperWave operation");
        const auto required=(kind==0||kind==9||kind==11)?128u:(kind==1||kind==10)?28u:344u;
        if (length!=required) throw std::runtime_error("Invalid SuperWave operation length");
        Bytes before; reader.read(before); const auto expected=reader.take(length);
        auto state=decode(before);
        if (kind>=12&&kind<=14) state=mfrk::SuperWaveState::admitted(state.words,state.dirty);
        mfrk::SuperWave engine(tables,state); mfrk::BasicWaveTables::Table explicit_table{};
        if (kind==11) reader.read(explicit_table);
        std::array<float,32> output{};
        switch(kind) {
        case 0: case 1: engine.render_raw(output); break;
        case 2: engine.set_pitch(hz,key); break;
        case 3: case 8: engine.initialize(); break;
        case 4: engine.prepare_controls(); break;
        case 5: engine.repoint(); break;
        case 6: engine.trigger(static_cast<std::uint8_t>(flag)); break;
        case 7: engine.prepare_controls(true); break;
        case 9: case 10: engine.render_scaled(output); break;
        case 11: mfrk::render_superwave32(state,explicit_table,output); break;
        case 12: break;
        case 13: engine.prepare_controls(); break;
        case 14: engine.initialize(); engine.prepare_controls(); break;
        case 15: engine.set_phase(hz); break;
        case 16: engine.set_phase(hz); engine.render_raw(output); engine.trigger(static_cast<std::uint8_t>(flag)); break;
        }
        const auto after=encode(engine.state(),before); const std::uint8_t* actual=after.data();
        if (kind==0||kind==9||kind==11) actual=reinterpret_cast<const std::uint8_t*>(output.data());
        if (kind==1||kind==10) actual=after.data()+0xa0;
        bool different=false;
        for (unsigned j=0;j<length;++j) {
            // Native opaque/unread padding is not a portable ABI requirement.
            if (length==344&&j>=0x110&&j<0x114) continue;
            if (actual[j]!=expected[j]) different=true;
        }
        ++cases[kind]; if (different) { if (failed<4) std::printf("Case %u kind %u mismatch\n",n,kind); ++failed; ++failures[kind]; }
    }
    if (reader.position!=reader.bytes.size()) throw std::runtime_error("Trailing canonical data");
    for (unsigned k=0;k<cases.size();++k) std::printf("SuperWave kind %u: %u cases, %u mismatches\n",k,cases[k],failures[k]);
    std::printf("Original SuperWave: %u fixtures, %u mismatches\n",count,failed);
    return failed?1:0;
} catch (const std::exception& error) { std::fprintf(stderr,"%s\n",error.what()); return 2; }
