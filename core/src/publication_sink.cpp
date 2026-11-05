#include "mfrk/publication_sink.hpp"
namespace mfrk::publication_sink {
bool publish(Owner& owner,std::uint8_t index,const notepath::NoteRow& row,std::uint8_t stack_byte) noexcept {
 if(index>=owner.rows.size()||owner.count>owner.queue.size())return false;
 owner.rows[index]=row;
 if(row.bytes[0]==1)owner.last_active=index;
 if(owner.count==owner.queue.size())return true;
 auto& packet=owner.queue[owner.count++];
 packet[0]=1;packet[1]=packet[2]=packet[3]=packet[4]=packet[5]=packet[6]=packet[7]=0;
 packet[8]=row.bytes[0]==1?0x90:0x80;packet[9]=row.bytes[1]&0x7f;packet[10]=row.bytes[3]&0x7f;packet[11]=stack_byte;
 return true;
}
bool publish_batch(Owner& owner,const notepath::Publications& batch,std::uint8_t stack_byte) noexcept {
 if(batch.count>batch.entries.size()||owner.count>owner.queue.size())return false;
 for(unsigned i=0;i<batch.count;++i)if(batch.entries[i].row>=owner.rows.size())return false;
 for(unsigned i=0;i<batch.count;++i)publish(owner,batch.entries[i].row,batch.entries[i].value,stack_byte);
 return true;
}
}
