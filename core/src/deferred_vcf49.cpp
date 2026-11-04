#include "mfrk/deferred_vcf49.hpp"
#include <algorithm>
namespace mfrk {
struct DeferredVcf49::Impl{};
DeferredVcf49::DeferredVcf49(std::uint32_t,bool):impl_(std::make_unique<Impl>()){}
DeferredVcf49::~DeferredVcf49()=default;
void DeferredVcf49::initialize(float){}
void DeferredVcf49::set_normalized(Vcf49Parameter,float) noexcept{}
void DeferredVcf49::set_physical(Vcf49Parameter,float) noexcept{}
float DeferredVcf49::normalized(Vcf49Parameter) noexcept{return 0;}
float DeferredVcf49::physical(Vcf49Parameter) noexcept{return 0;}
void DeferredVcf49::prepare(bool){}
bool DeferredVcf49::render(std::span<const float,32>,std::span<const float,32>,std::span<float,32> out) noexcept{std::fill(out.begin(),out.end(),0.0f);return true;}
void DeferredVcf49::end_block() noexcept{}
void DeferredVcf49::reset() noexcept{}
void DeferredVcf49::request_rebuild() noexcept{}
bool DeferredVcf49::worker_step(){return false;}
DeferredFilterState DeferredVcf49::state() const noexcept{return {};}
FilterParameterSnapshot DeferredVcf49::parameter_snapshot(std::uint32_t) const noexcept{return {};}
}
