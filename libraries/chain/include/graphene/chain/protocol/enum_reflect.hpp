/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <fc/reflect/reflect.hpp>

namespace graphene { namespace chain { namespace util {

template <typename T>
struct convert_enum
{
  static T from_string(const char* s) = delete;
};

template <typename T>
struct linkable_struct
{
  typedef T val_t;
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


#define UTIL_LINK_ENUM_GET_F( R, ENUM_TYPE, FIELD ) case ENUM_TYPE::FIELD: return FIELD;
#define UTIL_LINK_ENUM_SET_F( R, ENUM_TYPE, FIELD ) case ENUM_TYPE::FIELD: FIELD = val; break;

#define LINK_ENUM_TO_FIELDS( ENUM, FIELDS ) \
  val_t get(ENUM kind) const \
  { \
      switch(kind) \
      { \
          BOOST_PP_SEQ_FOR_EACH( UTIL_LINK_ENUM_GET_F, ENUM, FIELDS ) \
          default: \
              FC_THROW_EXCEPTION( fc::invalid_arg_exception, \
                                 "Class does not have field '${a}'", \
                                 ("a", fc::reflector<ENUM>::to_fc_string(kind)) \
                                ); \
      } \
      return val_t(); \
  } \
  void set(ENUM kind, val_t val) \
  { \
      switch(kind) \
      { \
          BOOST_PP_SEQ_FOR_EACH( UTIL_LINK_ENUM_SET_F, ENUM, FIELDS ) \
          default: \
             FC_THROW_EXCEPTION( fc::invalid_arg_exception, \
                                 "Class does not have field '${a}'", \
                                 ("a", fc::reflector<ENUM>::to_fc_string(kind)) \
                               ); \
      } \
  } \

