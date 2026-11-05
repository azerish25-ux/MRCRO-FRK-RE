#include "mfrk/preset_migration.hpp"
#include <bit>
#include <iostream>
using namespace mfrk::preset;
int main(){unsigned differences=0;
{
const std::string_view labels0[]={"A","Old"};
const std::string_view labels1[]={"A","Renamed"};
const Record records[]={Record{Version{0,0,0},labels0},Record{Version{1,0,0},labels1}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1065353216u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1048576000u;
}
{
const std::string_view labels0[]={"A","Gone","C"};
const std::string_view labels1[]={"A","C"};
const Record records[]={Record{Version{0,0,0},labels0},Record{Version{1,0,0},labels1}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1056964608u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1048576000u;
}
{
const std::string_view labels0[]={"A","B"};
const std::string_view labels1[]={"B","A","B"};
const Record records[]={Record{Version{0,0,0},labels0},Record{Version{1,0,0},labels1}};
Definition definition{records,3,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1065353216u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1065353216u;
}
{
const std::string_view labels0[]={"A","B"};
const std::string_view labels1[]={"B","A"};
const std::string_view labels2[]={"A","B"};
const Record records[]={Record{Version{2,0,0},labels0},Record{Version{0,0,0},labels1},Record{Version{3,0,0},labels2}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("2.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1058642330u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=0u;
}
{
const std::string_view labels0[]={"A","B"};
const std::string_view labels1[]={"A","B","C"};
const Record records[]={Record{Version{0,0,0},labels0},Record{Version{1,0,0},labels1}};
Definition definition{records,5,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1065353216u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1048576000u;
}
{
const std::string_view labels0[]={"Only"};
const Record records[]={Record{Version{0,0,0},labels0}};
Definition definition{records,1,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1056964608u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=0u;
}
{
const std::string_view labels0[]={"A","B"};
const Record records[]={Record{Version{1,0,0},labels0}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1056964608u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1048576000u;
}
{
const std::string_view labels0[]={"A",""};
const Record records[]={Record{Version{1,0,0},labels0}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1056964608u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1065353216u;
}
{
const std::string_view labels0[]={"","A"};
const std::string_view labels1[]={"A",""};
const Record records[]={Record{Version{0,0,0},labels0},Record{Version{1,0,0},labels1}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{0u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1065353216u;
}
{
const std::string_view labels0[]={"A","B"};
const Record records[]={Record{Version{0,0,0},labels0}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{3184315597u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=0u;
}
{
const std::string_view labels0[]={"A","B"};
const Record records[]={Record{Version{0,0,0},labels0}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{1066192077u})},false,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=1065353216u;
}
{
const std::string_view labels0[]={"A","B"};
const std::string_view labels1[]={"B","A"};
const Record records[]={Record{Version{0,0,0},labels0},Record{Version{1,0,0},labels1}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{true,std::bit_cast<float>(std::uint32_t{0u})},true,1.f);
differences+=result.present!=true||std::bit_cast<std::uint32_t>(result.normalized)!=0u;
}
{
const std::string_view labels0[]={"A","B"};
const Record records[]={Record{Version{0,0,0},labels0}};
Definition definition{records,2,.25f};
const auto result=convert_discrete(definition,parse_version("0.0.0"),Value{false,std::bit_cast<float>(std::uint32_t{1060840145u})},true,1.f);
differences+=result.present!=false||std::bit_cast<std::uint32_t>(result.normalized)!=1060840145u;
}
{
 const auto saved=select_numeric(42,true,0x7fc01234u,0x3f000000u,false,0);
 const auto missing=select_numeric(42,false,0,0x80000000u,false,0);
 const auto reset=select_numeric(42,true,0,0,true,0x7f800000u);
 differences+=saved.id!=42||saved.origin!=2||saved.tag!=1||saved.length!=4||saved.bits!=0x7fc01234u;
 differences+=missing.bits!=0x80000000u||reset.bits!=0x7f800000u;
}
std::cout<<differences;return differences?1:0;}
