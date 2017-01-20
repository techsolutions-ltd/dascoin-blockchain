/**
 * DASCOIN!
 */
#pragma once
#include <fc/reflect/reflect.hpp>

namespace graphene { namespace chain { namespace util {

template <typename T>
struct convert_enum{
  static T from_string(const char* s) = delete;
};

} } }  // namespace graphene::chain::util

#define REFLECT_ENUM_CHECK( ENUM, FIELDS ) \
   FC_REFLECT_ENUM( ENUM, FIELDS ) \
   namespace graphene { namespace chain { namespace util { \
       template<> struct convert_enum<ENUM> \
       { \
          static ENUM from_fc_string(fc::string s) { return from_cstring(s.c_str()); } \
          static ENUM from_string(std::string s) { return from_cstring(s.c_str()); } \
          static ENUM from_cstring(const char* s) \
          { \
            BOOST_PP_SEQ_FOR_EACH( FC_REFLECT_ENUM_FROM_STRING, ENUM, FIELDS ) \
            fc::throw_bad_enum_cast( s, BOOST_PP_STRINGIZE(ENUM) ); \
            return ENUM();\
          } \
       }; \
    }}}  \

