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

#include <graphene/chain/protocol/upgrade.hpp>

namespace graphene { namespace chain {

  void create_upgrade_event_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    FC_ASSERT( comment.size() <= DASCOIN_MAXIMUM_INTERNAL_MEMO_LENGTH );
    for (const auto& subsequent_event : subsequent_execution_times)
    {
      FC_ASSERT( execution_time < subsequent_event,
                 "Subsequent execution time cannot be before the main execution time, execution time: ${exec} subsequent time: ${time} ",
                 ("exec", execution_time)
                 ("time", subsequent_event)
               );
    }
  }

  void update_upgrade_event_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    if (comment.valid())
      FC_ASSERT( (*comment).size() <= DASCOIN_MAXIMUM_INTERNAL_MEMO_LENGTH );
    if (subsequent_execution_times.valid())
    {
      for (const auto& subsequent_event : *subsequent_execution_times)
      {
        FC_ASSERT( execution_time < subsequent_event,
                   "Subsequent execution time cannot be before the main execution time, execution time: ${exec} subsequent time: ${time} ",
                   ("exec", execution_time)
                   ("time", subsequent_event)
                 );
      }
    }
  }

  void delete_upgrade_event_operation::validate() const
  {

  }

} } // namespace graphene::chain
