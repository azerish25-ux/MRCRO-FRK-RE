#include "mfrk/preset_archive.hpp"
#include <charconv>
#include <bit>
#include <cmath>
#include <cstdlib>
#include <locale.h>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

namespace mfrk::preset_archive {
namespace {
bool whitespace(char c) noexcept{return c==' '||c=='\r'||c=='\n'||c=='\t';}
bool strip_space(char c) noexcept{return whitespace(c)||c=='\v'||c=='\f';}
bool digit(char c) noexcept{return c>='0'&&c<='9';}
bool alpha(char c) noexcept{return (c>='A'&&c<='Z')||(c>='a'&&c<='z');}
bool key_valid(std::string_view value) noexcept {
 if(value.empty()||(!alpha(value[0])&&value[0]!='_'))return false;
 for(char c:value)if(!alpha(c)&&!digit(c)&&c!='_')return false;
 return true;
}
bool utf8(std::string_view s) noexcept {
 for(std::size_t i=0;i<s.size();){
  const auto c=static_cast<unsigned char>(s[i++]);if(c<0x80)continue;
  unsigned n;std::uint32_t code,min;
  if(c>=0xc2&&c<=0xdf){n=1;code=c&0x1f;min=0x80;}
  else if(c>=0xe0&&c<=0xef){n=2;code=c&0x0f;min=0x800;}
  else if(c>=0xf0&&c<=0xf4){n=3;code=c&0x07;min=0x10000;}
  else return false;
  if(n>s.size()-i)return false;
  for(unsigned j=0;j<n;++j){const auto x=static_cast<unsigned char>(s[i++]);if((x&0xc0)!=0x80)return false;code=(code<<6)|(x&0x3f);}
  if(code<min||code>0x10ffff||(code>=0xd800&&code<=0xdfff))return false;
 }
 return true;
}
struct Cursor {
 std::string_view data;std::size_t pos=0;
 std::string_view token(){while(pos<data.size()&&whitespace(data[pos]))++pos;const auto start=pos;while(pos<data.size()&&!whitespace(data[pos]))++pos;if(start==pos)throw std::invalid_argument("missing token");return data.substr(start,pos-start);}
 std::int64_t integer(){auto t=token();if(t.front()=='+')t.remove_prefix(1);if(t.empty())throw std::invalid_argument("integer");std::int64_t x=0;auto result=std::from_chars(t.data(),t.data()+t.size(),x);if(result.ec!=std::errc{}||result.ptr!=t.data()+t.size())throw std::invalid_argument("integer");return x;}
 std::string_view counted(){const auto n=integer();if(n<0||pos>=data.size()||data[pos]!=' '||static_cast<std::uint64_t>(n)>data.size()-pos-1)throw std::invalid_argument("counted string");++pos;const auto result=data.substr(pos,static_cast<std::size_t>(n));pos+=static_cast<std::size_t>(n);return result;}
};
void require(bool value,const char* message){if(!value)throw std::invalid_argument(message);}
void append_counted(std::string& out,std::string_view text){out+=std::to_string(text.size());out+=' ';out.append(text);}
struct NumericLocale {
#ifdef _WIN32
 _locale_t value=_create_locale(LC_NUMERIC,"C");
 ~NumericLocale(){if(value)_free_locale(value);}
#else
 locale_t value=newlocale(LC_NUMERIC_MASK,"C",nullptr);
 ~NumericLocale(){if(value)freelocale(value);}
#endif
};
bool numeric(std::string_view text) {
 if(text.empty())return false;
 std::size_t p=0;if(text[p]=='+'||text[p]=='-')++p;unsigned digits=0;
 while(p<text.size()&&digit(text[p])){++p;++digits;}
 if(p<text.size()&&text[p]=='.'){++p;while(p<text.size()&&digit(text[p])){++p;++digits;}}
 if(!digits)return false;
 if(p<text.size()&&(text[p]=='e'||text[p]=='E')){++p;if(p<text.size()&&(text[p]=='+'||text[p]=='-'))++p;const auto start=p;while(p<text.size()&&digit(text[p]))++p;if(p==start)return false;}
 if(p!=text.size())return false;
 static const NumericLocale locale;if(!locale.value)return false;
 const std::string terminated(text);char* end=nullptr;
#ifdef _WIN32
 const double value=_strtod_l(terminated.c_str(),&end,locale.value);
#else
 const double value=strtod_l(terminated.c_str(),&end,locale.value);
#endif
 return end==terminated.c_str()+terminated.size()&&std::isfinite(value);
}
struct Candidate {std::size_t start=0,entries=0,end=0,slots_end=0;std::vector<Entry> properties;};
}
std::optional<std::uint32_t> numeric_bits(std::string_view text){
 if(!numeric(text))return std::nullopt;
 static const NumericLocale locale;if(!locale.value)return std::nullopt;
 const std::string terminated(text);char* end=nullptr;
#ifdef _WIN32
 const float value=_strtof_l(terminated.c_str(),&end,locale.value);
#else
 const float value=strtof_l(terminated.c_str(),&end,locale.value);
#endif
 if(end!=terminated.c_str()+terminated.size()||!std::isfinite(value))return std::nullopt;
 return std::bit_cast<std::uint32_t>(value);
}
View parse(std::span<const std::byte> input,bool allow_attachment_archive){
 require(!input.empty()&&input.size()<=64u*1024u*1024u,"archive size");
 const std::string_view data(reinterpret_cast<const char*>(input.data()),input.size());Cursor c{data};View view;view.raw=input;
 require(c.counted()=="serialization::archive"&&c.integer()==10,"archive signature/version");
 for(auto expected:{0,7,0,7})require(c.integer()==expected,"archive class profile");
 view.name=c.counted();view.bank=c.counted();view.product=c.integer();view.author=c.counted();view.unknown_header_text=c.counted();
 for(auto& slot:view.header_slots)slot=c.integer();view.description=c.counted();view.timestamp=c.integer();view.creator_version=c.counted();
 require(utf8(view.name)&&utf8(view.bank)&&utf8(view.author)&&utf8(view.unknown_header_text)&&utf8(view.description)&&utf8(view.creator_version),"header UTF-8");
 const auto header_end=c.pos;Candidate selected;unsigned found=0;
 for(std::size_t i=header_end;i<data.size();++i){
  if(data[i]!=' ')continue;std::size_t j=i+1;unsigned n=0,digits=0;
  while(j<data.size()&&digit(data[j])&&digits<4){n=n*10+unsigned(data[j]-'0');++j;++digits;}
  if(!digits||digits>3||data.substr(j,7)!=" 0 0 0 ")continue;const auto entries=j+7;std::size_t look=entries;
  while(look<data.size()&&digit(data[look]))++look;
  if(look==entries||look+1>=data.size()||data[look]!=' '||!alpha(data[look+1]))continue;
  try{
   require(n!=0,"empty metadata");Cursor probe{data,entries};Candidate candidate;candidate.start=i;candidate.entries=entries;candidate.properties.reserve(n);std::string_view last;
   for(unsigned k=0;k<n;++k){const auto key=probe.counted(),value=probe.counted();require(key_valid(key)&&(k==0||key>last)&&utf8(value),"metadata entry");last=key;candidate.properties.push_back({key,value});}
   candidate.end=probe.pos;for(auto expected:{0,0,0,7,0,0,0,0,0,0})require(probe.integer()==expected,"state class profile");candidate.slots_end=probe.pos;
   std::string encoded;for(std::size_t k=0;k<candidate.properties.size();++k){if(k)encoded+=' ';append_counted(encoded,candidate.properties[k].key);encoded+=' ';append_counted(encoded,candidate.properties[k].value);}
   require(data.substr(entries,candidate.end-entries)==encoded,"metadata representation");selected=std::move(candidate);++found;
  }catch(const std::invalid_argument&){}
 }
 require(found==1,"ambiguous metadata");view.properties=std::move(selected.properties);view.unassigned_header=data.substr(header_end,selected.start-header_end);view.unassigned_state=data.substr(selected.end,selected.slots_end-selected.end);c.pos=selected.slots_end;
 const auto n=c.integer(),item_version=c.integer();require(n>=0&&n<=10000&&item_version==0,"numeric map header");if(n)require(c.integer()==0&&c.integer()==0,"numeric pair class");const auto numeric_entries=c.pos;view.parameters.reserve(static_cast<std::size_t>(n));std::string_view last;
 for(std::int64_t i=0;i<n;++i){const auto key=c.counted(),value=c.token();require(key_valid(key)&&(i==0||key>last)&&numeric(value),"numeric entry");last=key;view.parameters.push_back({key,value});}
 const auto blob_count=c.integer(),blob_version=c.integer();require(blob_count>=0&&blob_count<=1000&&blob_version==0,"blob map header");view.blobs.reserve(static_cast<std::size_t>(blob_count));last={};
 for(std::int64_t i=0;i<blob_count;++i){const auto key=c.counted(),value=c.counted();require(key_valid(key)&&(i==0||key>last),"blob entry");last=key;view.blobs.push_back({key,value});}
 const auto blob_end=c.pos;view.trailing=data.substr(blob_end);auto stripped=view.trailing;while(!stripped.empty()&&strip_space(stripped.front()))stripped.remove_prefix(1);while(!stripped.empty()&&strip_space(stripped.back()))stripped.remove_suffix(1);
 require(stripped.empty()||(allow_attachment_archive&&stripped.starts_with("22 serialization::archive 10 ")),"archive trailing data");
 std::string encoded;for(const auto& entry:view.parameters){encoded+=' ';append_counted(encoded,entry.key);encoded+=' ';encoded.append(entry.value);}encoded+=' ';encoded+=std::to_string(blob_count);encoded+=" 0";for(const auto& entry:view.blobs){encoded+=' ';append_counted(encoded,entry.key);encoded+=' ';append_counted(encoded,entry.value);}
 require(data.substr(numeric_entries,blob_end-numeric_entries)==encoded,"map representation");return view;
}
Owned::Owned(std::span<const std::byte> input,bool attachments):storage_(input.begin(),input.end()),view_(parse(storage_,attachments)){}
Owned::Owned(std::vector<std::byte> input,bool attachments):storage_(std::move(input)),view_(parse(storage_,attachments)){}
Owned::Owned(Owned&& other) noexcept:storage_(std::move(other.storage_)),view_(std::move(other.view_)){other.view_=View{};}
Owned& Owned::operator=(Owned&& other) noexcept {if(this!=&other){storage_=std::move(other.storage_);view_=std::move(other.view_);other.view_=View{};}return *this;}
}
