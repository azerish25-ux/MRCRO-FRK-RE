#include "mfrk/preset_archive.hpp"
#include <stdexcept>
#include <string>
using namespace mfrk::preset_archive;
int main(){
 std::string source="22 serialization::archive 10 0 7 0 7 4 Test 4 Bank 66 1 A 1 U 0 0 0 0 1 0 0  0 0  0 1 0 0 0 3 Key 5 Value 0 0 0 7 0 0 0 0 0 0 1 0 0 0 3 Foo 0.5 1 0 4 Blob 3 ";source.append("a\0b",3);
 const auto view=parse(std::as_bytes(std::span(source)));
 if(view.name!="Test"||view.bank!="Bank"||view.product!=66||view.properties.size()!=1||view.properties[0].key!="Key"||view.properties[0].value!="Value"||view.parameters.size()!=1||view.parameters[0].key!="Foo"||view.parameters[0].value!="0.5"||view.blobs.size()!=1||view.blobs[0].key!="Blob"||view.blobs[0].value!=std::string_view("a\0b",3))return 1;
 auto reject=[](const std::string& text){try{parse(std::as_bytes(std::span(text)));return false;}catch(const std::invalid_argument&){return true;}};
 auto bad=source;bad[3]='X';if(!reject(bad))return 2;bad=source;bad.pop_back();if(!reject(bad))return 3;bad=source;bad+=" garbage";if(!reject(bad))return 4;
 std::string empty="22 serialization::archive 10 0 7 0 7 4 Init 4 Bank 66 1 A 1 U 0 0 0 0 1 0 0  0 0  0 1 0 0 0 3 Key 5 Value 0 0 0 7 0 0 0 0 0 0 0 0 0 0";
 auto init=parse(std::as_bytes(std::span(empty)));if(init.name!="Init"||!init.parameters.empty()||!init.blobs.empty())return 5;
 auto attached=source+"\n22 serialization::archive 10 opaque";if(!reject(attached))return 6;
 auto attachment=parse(std::as_bytes(std::span(attached)),true);if(attachment.trailing!="\n22 serialization::archive 10 opaque"||attachment.raw.size()!=attached.size())return 7;
 bad=source;bad.replace(bad.find("0.5"),3,"nan");if(!reject(bad))return 8;
}
