#include "mfrk/note_row_pitch.hpp"
namespace mfrk::note_row_pitch {
bool collect(Records& records,pitch_update::GlideControl& control,std::uint8_t& active,const std::array<notepath::NoteRow,12>& rows,bool para) noexcept {
 for(unsigned voice=0;voice<6;++voice)for(unsigned side=0;side<2;++side){const unsigned row=voice+side*6;const auto status=rows[row].bytes[7];if(status>5)return false;if(status==1||status==3||status==4){const auto pitch=rows[para?row:voice].bytes[1];if(pitch>127)return false;}}
 auto next=records;auto next_control=control;auto count=active;
 for(unsigned voice=0;voice<6;++voice)for(unsigned side=0;side<2;++side){
  const unsigned row=voice+side*6,index=para?row:voice;const auto status=rows[row].bytes[7];
  if(status==1||status==3||status==4){
   if(status==1)count=static_cast<std::uint8_t>(count+1);
   std::uint16_t word=0;pitch_update::encode_note_pitch(rows[para?row:voice].bytes[1],word);next.words[index]=word;next_control.dirty_a[index]=1;next_control.held[index]=0;
  }else if(status==2||status==5){next_control.held[index]=1;if(count)count=static_cast<std::uint8_t>(count-1);next_control.shared_gate=count==0?1:0;}
 }
 records=next;control=next_control;active=count;return true;
}
bool apply_pending(const Records& records,pitch_update::State& state,pitch_update::GlideControl& control) noexcept {
 auto next=state;auto next_control=control;
 for(unsigned voice=0;voice<6;++voice)for(unsigned side=0;side<2;++side){const unsigned index=voice+side*6;if(next_control.dirty_a[index]&&!pitch_update::apply_note_word(next,next_control,index,records.words[index]))return false;}
 state=next;control=next_control;return true;
}
void clear_note_dirty(pitch_update::GlideControl& control) noexcept {control.dirty_a={};}
}
