#include "mfrk/slot_pitch.hpp"
#include <bit>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
namespace mfrk::slot_pitch {struct TestAccess{static const auto&state(const Pitch&p){return p.state_;}};}
template<class T>T read(std::istream&in){T x{};if(!in.read(reinterpret_cast<char*>(&x),sizeof x))throw std::runtime_error("truncated");return x;}
int main(int argc,char**argv)try{
 if(argc!=2)throw std::runtime_error("arguments");
 std::ifstream in(argv[1],std::ios::binary);std::string magic(8,' ');in.read(magic.data(),8);if(magic!="MFSPIT01")throw std::runtime_error("magic");
 const auto count=read<std::uint32_t>(in);if(!count||count>100000)throw std::runtime_error("count");auto p=mfrk::slot_pitch::Pitch::admitted_zero_owner_arena();unsigned state_errors=0,updated_errors=0,result_errors=0;
 const auto initial=read<std::array<std::uint8_t,112>>(in);if(initial!=mfrk::slot_pitch::TestAccess::state(p))throw std::runtime_error("constructor");
 for(unsigned n=0;n<count;++n){mfrk::slot_pitch::Input input{};input.base_note=std::bit_cast<float>(read<std::uint32_t>(in));input.words=read<std::array<std::uint16_t,9>>(in);input.dirty=read<std::array<std::uint8_t,9>>(in);input.force=read<std::uint8_t>(in);const auto expected_updated=read<std::uint8_t>(in);if(expected_updated>1)throw std::runtime_error("updated");const auto expected=read<std::array<std::uint8_t,112>>(in);mfrk::slot_pitch::Result result{};if(!p.try_prepare(input,result))throw std::runtime_error("unexpected admission");updated_errors+=result.recomputed!=(expected_updated!=0);std::uint32_t expected_note,expected_hz;std::memcpy(&expected_note,expected.data()+0x58,4);std::memcpy(&expected_hz,expected.data()+0x5c,4);result_errors+=std::bit_cast<std::uint32_t>(result.note)!=expected_note;result_errors+=std::bit_cast<std::uint32_t>(result.hz)!=expected_hz;for(unsigned i=0;i<112;++i)if(mfrk::slot_pitch::TestAccess::state(p)[i]!=expected[i]){if(state_errors<12)std::cerr<<"case "<<n<<" offset "<<std::hex<<i<<" expected "<<unsigned(expected[i])<<" got "<<unsigned(mfrk::slot_pitch::TestAccess::state(p)[i])<<std::dec<<'\n';++state_errors;}}
 if(in.peek()!=EOF)throw std::runtime_error("trailing");
 unsigned admission=0;const auto before=mfrk::slot_pitch::TestAccess::state(p);for(auto bits:{0x7f800000u,0xff800000u,0x7fc00000u}){mfrk::slot_pitch::Input input{};input.base_note=std::bit_cast<float>(bits);mfrk::slot_pitch::Result result{true,123.,456.};admission+=p.try_prepare(input,result);admission+=before!=mfrk::slot_pitch::TestAccess::state(p);admission+=!result.recomputed||result.note!=123.||result.hz!=456.;}
 std::cout<<"cases "<<count<<" statebytes "<<state_errors<<" updated "<<updated_errors<<" results "<<result_errors<<" admission "<<admission<<'\n';return state_errors||updated_errors||result_errors||admission?1:0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
