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

#include <graphene/chain/protocol/license.hpp>

namespace graphene { namespace chain {

  void create_license_type_operation::validate() const
  {

  }

  void edit_license_type_operation::validate() const
  {
    if (name.valid())
    {
      FC_ASSERT( name->size() <= DASCOIN_MAX_LICENSE_NAME_LEN, "License's name cannot be longer than ${len}", ("len", DASCOIN_MAX_LICENSE_NAME_LEN) );
      FC_ASSERT( name->size() > 0, "Cannot set license's name to an empty string" );
    }
    if (amount.valid())
    {
      FC_ASSERT( amount->value > 0, "Cannot set cycle amount to negative or zero number ${amount}", ("amount", *amount) );
    }
    if (eur_limit.valid())
    {
      FC_ASSERT( eur_limit->value > 0, "Cannot set eur limit to negative or zero number ${amount}", ("amount", *eur_limit) );
    }
  }

  void issue_license_operation::validate() const
  {
    FC_ASSERT( frequency_lock >= 0 );  // NOTE: for charter licenses, must not be 0.
    FC_ASSERT( bonus_percentage > -100,
               "Illegal bonus percentage ${b}, value would lead to negative amount of cycles",
               ("b", bonus_percentage)
             );
  }

  void update_license_operation::validate() const
  {

  }

} } // namespace graphene::chain
