#include "mfrk/preset_bank.hpp"
#include "ui_scene_scope_fixture_reader.hpp"
constexpr std::size_t typed_value_fixture_size=425260;

#include <algorithm>
#include <bit>
#include <fstream>
#include <iostream>
namespace mfrk::preset_bank {struct StoreTestAccess {static void observe(void(*f)(const void*)){Value::free_observer_=f;}};}
using namespace mfrk::preset_bank;
static_assert(std::endian::native==std::endian::little);
static std::vector<std::uintptr_t>freed;
static void on_free(const void*p){freed.push_back(reinterpret_cast<std::uintptr_t>(p));}
int main(int argc,char**argv)try{
 if(argc!=2)return 2;
 std::ifstream f(argv[1],std::ios::binary|std::ios::ate);if(!f||f.tellg()!=static_cast<std::streamoff>(typed_value_fixture_size))return 2;std::vector<std::uint8_t>b(typed_value_fixture_size);f.seekg(0);if(!f.read(reinterpret_cast<char*>(b.data()),b.size())||std::memcmp(b.data(),"PVALOWN1",8))return 2;if(mfrk::ui::scene::scope::test::sha256(b)!=std::array<std::uint8_t,32>{0x47,0x79,0x26,0x6c,0xae,0x40,0xfb,0xd7,0x62,0xcd,0xc7,0xf4,0xff,0x53,0xa0,0xc3,0xa7,0xdd,0x00,0xbd,0x8d,0x2d,0x6b,0x50,0x9e,0x1a,0x89,0x8c,0xcf,0xf7,0x95,0xd1})return 2;std::size_t p=8;auto read=[&]<class T>(){if(p>b.size()||sizeof(T)>b.size()-p)throw std::runtime_error("truncated");T v;std::memcpy(&v,b.data()+p,sizeof v);p+=sizeof v;return v;};auto take=[&](std::size_t n){if(p>b.size()||n>b.size()-p)throw std::runtime_error("truncated");auto s=std::span<const std::uint8_t>(b).subspan(p,n);p+=n;return s;};if(read.operator()<std::uint32_t>()!=94)return 2;unsigned differences=0;StoreTestAccess::observe(on_free);
 struct Logical{std::uint32_t tag;std::vector<std::uint8_t>bytes;};auto logical=[](const Value&v){auto s=v.bytes();return Logical{v.tag(),{s.begin(),s.end()}};};
 for(unsigned i=0;i<94;++i){auto scalar=read.operator()<std::uint8_t>(),oldheap=read.operator()<std::uint8_t>();auto bits=read.operator()<std::uint32_t>();auto n=read.operator()<std::uint16_t>();if(scalar>1||oldheap>1||n>8192)return 2;auto input=take(n);Value src=scalar?Value::scalar(bits):Value::data(input);const auto original=logical(src);const auto src_heap=src.tag()==2?reinterpret_cast<std::uintptr_t>(src.data_address()):0;std::vector<std::uint8_t>oldbytes;for(unsigned j=0;j<5;++j){const std::string x="previous-destination";oldbytes.insert(oldbytes.end(),x.begin(),x.end());}Value dst=oldheap?Value::data(oldbytes):Value::scalar(0x3f000001);const auto old=dst.tag()==2?reinterpret_cast<std::uintptr_t>(dst.data_address()):0;freed.clear();dst=src;const auto copied=logical(dst);bool independent=!src_heap||reinterpret_cast<std::uintptr_t>(dst.data_address())!=src_heap;bool copy_freed=old&&std::find(freed.begin(),freed.end(),old)!=freed.end();const auto copy_heap=dst.tag()==2?reinterpret_cast<std::uintptr_t>(dst.data_address()):0;freed.clear();dst=std::move(src);const auto moved=logical(dst),reset=logical(src);bool transferred=!src_heap||reinterpret_cast<std::uintptr_t>(dst.data_address())==src_heap;bool move_freed=copy_heap&&std::find(freed.begin(),freed.end(),copy_heap)!=freed.end();const auto before_self=logical(dst);[](Value&target,Value&source){target=std::move(source);}(dst,dst);bool self_same=dst.tag()==before_self.tag&&logical(dst).bytes==before_self.bytes;Value ctor(std::move(dst));const auto constructed=logical(ctor);freed.clear();ctor=Value::scalar(0);bool final_freed=src_heap&&std::find(freed.begin(),freed.end(),src_heap)!=freed.end();std::array<Logical,5>actual{original,copied,moved,reset,constructed};bool bad=false;
  for(const auto&v:actual){auto tag=read.operator()<std::uint8_t>();auto size=read.operator()<std::uint16_t>();if(size>8192||(tag!=1&&tag!=2&&tag!=3))return 2;auto expected=take(size);bad|=tag!=v.tag||size!=v.bytes.size()||!std::equal(v.bytes.begin(),v.bytes.end(),expected.begin(),expected.end());}
  for(bool v:{independent,copy_freed,transferred,move_freed,final_freed,self_same}){auto expected=read.operator()<std::uint8_t>();if(expected>1)return 2;bad|=v!=bool(expected);}differences+=bad;
 }
 StoreTestAccess::observe(nullptr);if(p!=b.size())return 2;std::cout<<"{\"cases\":94,\"differences\":"<<differences<<"}\n";return differences?1:0;
}catch(const std::exception&e){StoreTestAccess::observe(nullptr);std::cerr<<e.what();return 2;}
