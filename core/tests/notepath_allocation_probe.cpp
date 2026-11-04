#include "notepath_allocation_probe.hpp"
#include <cstdlib>
#include <new>
namespace { bool watching=false;std::size_t allocation_count=0; }
namespace notepath_test_allocations {
void begin() noexcept {allocation_count=0;watching=true;}
std::size_t end() noexcept {watching=false;return allocation_count;}
}
void* operator new(std::size_t size){
  if(watching)++allocation_count;
  if(void* pointer=std::malloc(size?size:1))return pointer;
  throw std::bad_alloc();
}
void* operator new[](std::size_t size){return ::operator new(size);}
void operator delete(void* pointer) noexcept {std::free(pointer);}
void operator delete[](void* pointer) noexcept {std::free(pointer);}
void operator delete(void* pointer,std::size_t) noexcept {std::free(pointer);}
void operator delete[](void* pointer,std::size_t) noexcept {std::free(pointer);}
