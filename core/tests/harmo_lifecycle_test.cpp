#include "harmo_test_access.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <stdexcept>
using namespace mfrk::harmo;
static_assert(std::endian::native==std::endian::little);
int main(int argc,char** argv) try {
    if(argc!=2)return 2;
    std::ifstream input(argv[1],std::ios::binary|std::ios::ate);
    if(!input || input.tellg()!=30828)return 2;
    std::vector<std::uint8_t> bytes(30828);input.seekg(0);
    if(!input.read(reinterpret_cast<char*>(bytes.data()),bytes.size()))return 2;
    if(std::memcmp(bytes.data(),"HARMOL01",8))return 2;
    std::size_t position=8;
    auto read=[&]<class T>() {
        if(sizeof(T)>bytes.size()-position)throw std::runtime_error("truncated lifecycle fixture");
        T value;std::memcpy(&value,bytes.data()+position,sizeof value);
        position+=sizeof value;return value;
    };
    const auto count=read.operator()<std::uint32_t>();if(count!=36)return 2;
    Tables tables;auto engine=Engine::admitted_zero_arena(tables,true);
    unsigned mismatches=0;
    for(unsigned i=0;i<count;++i) {
        const auto kind=read.operator()<std::uint32_t>();
        const auto value=read.operator()<std::uint32_t>();
        TestAccess::state(engine)=read.operator()<std::array<std::uint8_t,424>>();
        switch(kind) {
            case 0:engine.set_phase(std::bit_cast<float>(value));break;
            case 1:engine.set_frequency(std::bit_cast<float>(value));break;
            case 2:engine.trigger(value!=0);break;
            case 3:engine.bind_buffer((value&1)!=0,value>>1);break;
            case 4:engine.initialize();break;
            default:return 2;
        }
        const auto expected=read.operator()<std::array<std::uint8_t,424>>();
        for(unsigned j=0;j<424;++j)mismatches+=TestAccess::state(engine)[j]!=expected[j];
    }
    if(position!=bytes.size())return 2;
    std::cout<<"{\"commands\":"<<count<<",\"byte_mismatches\":"<<mismatches<<"}\n";
    return mismatches?1:0;
} catch(const std::exception& error) {
    std::cerr<<error.what()<<'\n';return 2;
}
