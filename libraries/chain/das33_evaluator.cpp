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

#include <graphene/chain/das33_evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

  void_result das33_pledge_cycles_evaluator::do_evaluate(const das33_pledge_cycles_operation& op)
  { try {

      FC_ASSERT( false, "Not implemented" );
      return {};

    } FC_CAPTURE_AND_RETHROW((op)) }

  void_result das33_pledge_cycles_evaluator::do_apply(const das33_pledge_cycles_operation& op)
  { try {

    FC_ASSERT( false, "Not implemented" );
    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
