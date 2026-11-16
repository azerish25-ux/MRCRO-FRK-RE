#include "mfrk/preset_migration.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
constexpr std::size_t ordinary_data_fixture_size=55837;
#include <bit>
#include <cstring>
#include <fstream>
#include <iostream>
using namespace mfrk::preset;
static_assert(std::endian::native==std::endian::little);
int main(int argc,char**argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=static_cast<std::streamoff>(ordinary_data_fixture_size))return 2;
 std::vector<std::uint8_t>b(ordinary_data_fixture_size);f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),b.size())||std::memcmp(b.data(),"DAPPLY01",8))return 2;std::size_t p=8;
 if(mfrk::ui::scene::scope::test::sha256(b)!=std::array<std::uint8_t,32>{0x04,0x78,0x46,0x12,0x9f,0x40,0xc2,0xc9,0xcf,0xd1,0x1a,0xf3,0x78,0x3e,0x4d,0x03,0x62,0x2c,0x85,0x03,0x3d,0x59,0xa4,0xbd,0xe9,0x9f,0xbf,0x9b,0x19,0x1b,0x3e,0x79})return 2;
 auto read=[&]<class T>(){if(p>b.size()||sizeof(T)>b.size()-p)throw std::runtime_error("truncated");T v;std::memcpy(&v,b.data()+p,sizeof v);p+=sizeof v;return v;};auto take=[&](std::size_t n){if(p>b.size()||n>b.size()-p)throw std::runtime_error("truncated");auto s=std::span<const std::uint8_t>(b).subspan(p,n);p+=n;return s;};if(read.operator()<std::uint32_t>()!=675)return 2;unsigned differences=0;
 for(unsigned i=0;i<675;++i){const auto present=read.operator()<std::uint8_t>(),reset=read.operator()<std::uint8_t>(),flex=read.operator()<std::uint8_t>(),inl=read.operator()<std::uint8_t>();const auto dn=read.operator()<std::uint16_t>(),sn=read.operator()<std::uint16_t>(),en=read.operator()<std::uint16_t>();const auto tag=read.operator()<std::uint8_t>();if(present>1||reset>1||flex>1||inl>1||dn>512||sn>512||en>512||(inl&&dn>16)||(tag!=2&&tag!=3))return 2;auto fallback=take(dn),saved=take(sn),expected=take(en);auto result=apply_data(fallback,present?std::optional(saved):std::nullopt,reset,flex);differences+=!result||result->tag!=tag||!result->dirty||result->payload.size()!=en||!std::equal(result->payload.begin(),result->payload.end(),expected.begin(),expected.end());}
 if(p!=b.size())return 2;
 std::vector<std::uint8_t> oversized(513);if(apply_data(oversized,std::nullopt,false,false)||apply_data({},std::span<const std::uint8_t>(oversized),false,true))return 2;
 std::cout<<"{\"cases\":675,\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception&e){std::cerr<<e.what();return 2;}
