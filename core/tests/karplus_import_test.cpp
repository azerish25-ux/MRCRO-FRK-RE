#include "karplus_test_access.hpp"
#include "ucrt_sinf.hpp"
#include "ucrt_exp2f_compat.hpp"
#include "karplus_fixture_reader.hpp"
#include <iostream>
int main(int argc,char**argv)try{if(argc!=2)return 2;FixtureReader r(argv[1]);if(r.read<std::array<char,8>>()!=std::array<char,8>{'K','R','P','M','0','0','0','1'})return 2;const auto n=r.read<std::uint32_t>();if(n==0||n>100000)return 2;unsigned errors=0;for(unsigned i=0;i<n;++i){const auto kind=r.read<std::uint8_t>();const auto x=F(r.read<std::uint32_t>());const auto expected=r.read<std::uint32_t>();float value=0;if(kind==0){if(!mfrk::detail::ucrt_sinf::in_domain(x))return 2;
#ifdef MUTANT_DOUBLE_SIN
value=static_cast<float>(std::sin(static_cast<double>(x)));
#else
value=mfrk::detail::ucrt_sinf::evaluate(x);
#endif
}else if(kind==1){if(!std::isfinite(x)||x< -64||x>64)return 2;value=mfrk::detail::ucrt_exp2f::evaluate(x);}else return 2;errors+=std::bit_cast<std::uint32_t>(value)!=expected;}r.end();std::cout<<"original import returns "<<n<<" errors "<<errors<<"\n";return errors?1:0;}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 2;}
