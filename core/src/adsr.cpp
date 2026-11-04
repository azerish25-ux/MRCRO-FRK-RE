#include "mfrk/adsr.hpp"
#include "adsr_maps.hpp"
#include <algorithm>
#include <cassert>
#include <bit>
#include <cmath>
#include <limits>
#include <utility>
namespace mfrk {
  namespace {
    using Curve=std::array<std::uint8_t,0x710>;
    std::int16_t w16(std::int64_t v) noexcept {
      return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(v));
    }
    std::int32_t w32(std::int64_t v) noexcept {
      return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(v));
    }
    std::int64_t signed64(std::uint64_t v) noexcept {
      return std::bit_cast<std::int64_t>(v);
    }
    std::int16_t sat16(std::int64_t v) noexcept {
      return static_cast<std::int16_t>(std::clamp<std::int64_t>(v,-32768,32767));
    }
    std::int32_t sat32(std::int64_t v) noexcept {
      return static_cast<std::int32_t>(std::clamp<std::int64_t>(v,-2147483648LL,2147483647));
    }
    std::int16_t mul15(std::int16_t a,std::int16_t b) noexcept {
      if (a==32767)return b;
      if (b==32767)return a;
      return w16((std::int32_t(a)*b)>>15);
    }
    void put32(Curve& c,unsigned o,std::int64_t value) noexcept {
      auto u=static_cast<std::uint32_t>(value);
      for (unsigned i=0;i<4;++i)c[o+i]=static_cast<std::uint8_t>(u>>(i*8));
    }
    void put16(Curve& c,unsigned o,std::int64_t value) noexcept {
      auto u=static_cast<std::uint16_t>(value);
      c[o]=static_cast<std::uint8_t>(u);
      c[o+1]=static_cast<std::uint8_t>(u>>8);
    }
    std::int32_t get32(std::span<const std::uint8_t,0x710> c,unsigned o) noexcept {
      std::uint32_t u=0;
      for (unsigned i=0;i<4;++i)u|=std::uint32_t(c[o+i])<<(i*8);
      return std::bit_cast<std::int32_t>(u);
    }
    std::int16_t get16(std::span<const std::uint8_t,0x710> c,unsigned o) noexcept {
      return w16(std::uint16_t(c[o])|(std::uint16_t(c[o+1])<<8));
    }
    std::int16_t short_lookup(std::span<const std::int16_t,33> values,std::int16_t time) noexcept {
      auto index=(std::uint16_t(time)>>10)&31u;
      auto fraction=std::uint16_t(time)&1023u;
      return w16(values[index]+((std::int32_t(values[index+1]-values[index])*std::int32_t(fraction))>>10));
    }
    Curve generate_curve(unsigned variant) {
      Curve c{};
      put32(c,0x408,variant?0xdcc407:0x1c8f8772);
      for (unsigned i=0;i<128;++i) {
        const double power=std::pow(10.0,double(i)*-0.03999355311848523);
        std::int64_t attack,decay,offset;
        std::int16_t auxiliary;
        if (variant) {
          const double e0=std::exp(power*-1.6689051161630393),e1=std::exp(power*-2.333637155484591);
          attack=static_cast<std::int64_t>((e1-e0)*262152.0002441481+e0*2147483648.0);
          decay=static_cast<std::int64_t>(std::exp(power*-3.3333484662997384)*2147483648.0);
          offset=0x17cd7;
          auxiliary=0x7624;
        }else {
          const double factor=power*-0.3333333333333333;
          attack=static_cast<std::int64_t>(std::exp(factor*1.7014132779827524)*2147483648.0);
          const auto mapped=short_lookup(detail::adsr::decay_mapping,static_cast<std::int16_t>(i*256));
          const auto shape=short_lookup(detail::adsr::curve_a,mapped);
          auxiliary=short_lookup(detail::adsr::curve_b,mapped);
          const auto segment=shape>=32764?3:shape/8191;
          const double fraction=shape==32767?1.0:double((shape%8191)*4)*3.051850947599719e-05;
          const double lo=std::exp(std::log(1.0/std::exp(detail::adsr::curve_shape_exp_knots[segment])+1.0)*factor);
          const double hi=std::exp(std::log(1.0/std::exp(detail::adsr::curve_shape_exp_knots[segment+1])+1.0)*factor);
          decay=static_cast<std::int64_t>(((hi-lo)*fraction+lo)*2147483648.0);
          const double low=std::exp(detail::adsr::curve_shape_exp_knots[segment]),high=std::exp(detail::adsr::curve_shape_exp_knots[segment+1]);
          offset=static_cast<std::int64_t>(((high-low)*fraction+low)*2147483648.0);
        }
        put32(c,8+i*4,attack);
        put32(c,0x208+i*4,decay);
        put32(c,0x40c+i*4,offset);
        put16(c,0x60c+i*2,auxiliary);
      }
      return c;
    }
    std::int32_t interpolate(std::span<const std::uint8_t,0x710> c,unsigned offset,std::int16_t time) noexcept {
      const auto index=(std::uint16_t(time)>>8)&127u,fraction=std::uint16_t(time)&255u;
      const auto lo=get32(c,offset+index*4),hi=get32(c,offset+(index+1)*4);
      return w32(std::int64_t(lo)+(w32(std::int64_t(w32(std::int64_t(hi)-lo))*fraction)>>8));
    }
    std::int16_t auxiliary(unsigned variant,std::int16_t time) noexcept {
      auto c=Adsr::curve(variant);
      auto index=(std::uint16_t(time)>>8)&127u,fraction=std::uint16_t(time)&255u;
      auto lo=get16(c,0x60c+index*2),hi=get16(c,0x60c+(index+1)*2);
      return w16(lo+((std::int32_t(hi-lo)*std::int32_t(fraction))>>8));
    }
    std::int16_t convert_time(unsigned variant,std::int16_t time) noexcept {
      return short_lookup(variant?detail::adsr::percussive_mapping:detail::adsr::decay_mapping,time);
    }
    constexpr std::array<std::array<std::int16_t,3>,2> constants {
      {
        {
          786,-4521,155
        }, {
          5046,-3276,-28
        }
      }
    };
    std::int16_t effective_time(std::int16_t time,std::int16_t aux,std::int16_t dynamic,std::int16_t c0,std::int16_t c1) noexcept {
      if (aux<=0)return time;
      auto shift=mul15(sat16(std::int32_t(c0)+5887),dynamic),base=sat16(std::int32_t(c1)+4915);
      auto shifted=static_cast<std::int16_t>(std::clamp(std::int32_t(time)-shift,0,32767));
      auto blend=sat16(std::int32_t(base)+mul15(w16(32767-base),shifted));
      return sat16(std::int32_t(mul15(w16(32767-aux),time))+mul15(aux,blend));
    }
    std::pair<std::int32_t,std::int64_t> coefficients(unsigned variant,std::int16_t time,std::int16_t aux,std::int16_t dynamic,const std::array<std::int16_t,3>& constants,bool decay,std::int16_t sustain=0) noexcept {
      const auto t=effective_time(time,aux,dynamic,constants[0],constants[1]);
      auto c=Adsr::curve(variant);
      auto a=interpolate(c,0x208,t),off=interpolate(c,0x40c,t);
      const std::int32_t target=decay?w32(std::int64_t(sustain)*65536-off):w32(-std::int64_t(off));
      const auto difference=std::uint32_t{0x80000000u} -static_cast<std::uint32_t>(a);
      return {
        a,signed64(static_cast<std::uint64_t>(target)*difference)
      };
    }
  }
  Adsr::Adsr() noexcept { (void)curve(0); }

  std::span<const std::uint8_t,0x710> Adsr::curve(std::uint32_t variant) noexcept {
    static const std::array<Curve,2> curves {
      generate_curve(0),generate_curve(1)
    };
    assert(variant < curves.size());
    return curves[variant];
  }
  void Adsr::prepare(const AdsrControls& words,std::uint16_t mask) noexcept {
    controls_=words;
    mask&=0x1ff;
    auto& s=state_;
    auto norm=[&](unsigned i) {
      return float(words[i])*std::bit_cast<float>(std::uint32_t {
        0x37800200
      });
    };
    auto decay=[&] {
      s.decay_time=convert_time(s.decay_variant,w16(static_cast<std::int32_t>(norm(1)*28900.0f)));
      s.decay_aux=auxiliary(s.decay_variant,s.decay_time);
    };
    auto release=[&] {
      const float scaled=norm(3)*26587.80078125f;
      const float shifted=scaled+2312.199951171875f;
      s.release_time=convert_time(s.release_variant,w16(static_cast<std::int32_t>(shifted)));
      s.release_aux=auxiliary(s.release_variant,s.release_time);
    };
    if (mask&1) {
      const float powered=std::pow(norm(0),0.68f);
      s.attack_time=w16(static_cast<std::int32_t>(powered*27546.0f));
    }
    if (mask&2)decay();
    if (mask&4)s.sustain=w16(static_cast<std::int32_t>(norm(2)*32767.0f));
    if (mask&8)release();
    if (mask&16)s.attack_variant=(std::uint32_t(words[4])*2)>>16;
    if (mask&32) {
      s.decay_variant=(std::uint32_t(words[5])*2)>>16;
      s.decay_constants=constants[s.decay_variant];
      decay();
    }
    if (mask&64) {
      s.release_variant=(std::uint32_t(words[6])*2)>>16;
      s.release_constants=constants[s.release_variant];
      release();
    }
    if (mask&128)s.legato=((std::uint32_t(words[7])*2)>>16)==1;
    if (mask&256)s.mode=(std::uint32_t(words[8])*2)>>16;
    prepared_mask_|=mask;
  }
  void Adsr::reset() noexcept {
    state_.accumulator=0;
    state_.current=0;
    state_.phase=AdsrPhase::idle;
    state_.hold=state_.pending_release=0;
  }
  void Adsr::release_coefficients() noexcept {
    auto& s=state_;
    auto [a,b]=coefficients(s.release_variant,s.release_time,s.release_aux,s.release_dynamic,s.release_constants,false);
    s.release_coefficient=a;
    s.release_additive=b;
  }
  void Adsr::advance(std::int32_t a,std::int64_t b) noexcept {
    auto product=std::int64_t(a)*state_.accumulator;
    auto sum=static_cast<std::uint64_t>(product)+static_cast<std::uint64_t>(b);
    state_.accumulator=sat32(signed64(sum)>>31);
  }
  void Adsr::sustain_value() noexcept {
    state_.accumulator=w32(std::int64_t(state_.sustain)*65536);
    state_.current=state_.sustain;
  }
  bool Adsr::gate(bool on) noexcept {
    if (!prepared()) return false;
    auto& s = state_;
    if (!on) {
      if (s.hold) {
        s.pending_release = 1;
        return true;
      }
      if (static_cast<std::uint32_t>(s.phase) > 1) {
        if (s.accumulator > 0) {
          s.phase = AdsrPhase::release;
          s.release_dynamic = 32767;
          return true;
        }
        s.phase = AdsrPhase::idle;
      }
      return true;
    }

    s.trigger = 1;
    if (s.phase == AdsrPhase::forced_maximum ||
        (s.legato && s.phase != AdsrPhase::idle && s.phase != AdsrPhase::release)) {
      return true;
    }
    s.pending_release = 0;
    const auto prior = s.phase;
    release_coefficients();
    if (s.mode != 1) {
      s.accumulator = 0;
      s.current = 0;
    }
    s.retrigger_ticks = 0;
    s.phase = (prior == AdsrPhase::idle || s.mode == 1) ? AdsrPhase::attack : AdsrPhase::retrigger;
    return true;
  }

  bool Adsr::step() noexcept {
    if (!prepared()) return false;
    auto& s = state_;
    s.trigger = 0;
    switch (s.phase) {
      case AdsrPhase::idle:
        if (s.mode != 1) s.accumulator = s.current = 0;
        break;

      case AdsrPhase::forced_maximum:
        s.accumulator = 0x7fffffff;
        s.current = 32767;
        break;

      case AdsrPhase::attack: {
        const auto data = curve(s.attack_variant);
        s.attack_coefficient = interpolate(data, 8, s.attack_time);
        const auto target = get32(data, 0x408);
        const auto difference = std::uint32_t{0x80000000u} -
                                static_cast<std::uint32_t>(s.attack_coefficient);
        s.attack_additive = signed64((std::uint64_t(target) + 0x80000000ULL) * difference);
        advance(s.attack_coefficient, s.attack_additive);
        s.current = s.attack_time < 2756 ? 32767 : w16(s.accumulator >> 16);
        if (static_cast<std::uint32_t>(s.accumulator) > 0x7ffffffeu || s.attack_time < 1) {
          s.current = 32767;
          s.accumulator = 0x7fffffff;
          s.phase = AdsrPhase::decay;
          s.decay_dynamic = 32767;
        }
        break;
      }

      case AdsrPhase::decay: {
        const auto [a, b] = coefficients(
            s.decay_variant, s.decay_time, s.decay_aux, s.decay_dynamic,
            s.decay_constants, true, s.sustain);
        s.decay_coefficient = a;
        s.decay_additive = b;
        s.decay_dynamic = s.decay_dynamic < 0 ? 0 :
            w16(std::int32_t(s.decay_dynamic) - s.decay_constants[2] - 128);
        advance(a, b);
        if (s.decay_time > 2519) {
          s.current = w16(s.accumulator >> 16);
          if (s.sustain < s.current) break;
        }
        sustain_value();
        s.phase = AdsrPhase::sustain;
        break;
      }

      case AdsrPhase::sustain:
        sustain_value();
        break;

      case AdsrPhase::release:
        release_coefficients();
        if (s.hold) {
          sustain_value();
          break;
        }
        s.release_dynamic = s.release_dynamic < 0 ? 0 :
            w16(std::int32_t(s.release_dynamic) - s.release_constants[2] - 128);
        advance(s.release_coefficient, s.release_additive);
        if (s.release_time > 2519) {
          s.current = w16(s.accumulator >> 16);
          if (s.current > 0) break;
        }
        s.phase = AdsrPhase::idle;
        s.accumulator = s.current = 0;
        break;

      case AdsrPhase::retrigger:
        release_coefficients();
        ++s.retrigger_ticks;
        advance(s.release_coefficient, s.release_additive);
        s.current = s.release_time < 2520 ? 0 : w16(s.accumulator >> 16);
        if (s.accumulator < 1 || s.release_time < 1) s.accumulator = s.current = 0;
        if (s.retrigger_ticks >= 5) {
          s.phase = AdsrPhase::attack;
          if (s.mode != 1) s.accumulator = s.current = 0;
        }
        break;
    }
    s.output = s.current;
    return true;
  }
}
