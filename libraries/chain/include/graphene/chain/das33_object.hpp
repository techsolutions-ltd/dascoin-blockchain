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

#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

  ///////////////////////////////
  // OBJECTS:                  //
  ///////////////////////////////

  class das33_project_object : public graphene::db::abstract_object<das33_project_object>
  {

  };

  class das33_cycles_pledge_holder_object : public abstract_object<das33_cycles_pledge_holder_object>
  {

  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

