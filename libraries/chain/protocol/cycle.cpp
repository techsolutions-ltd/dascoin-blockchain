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

#include <graphene/chain/protocol/cycle.hpp>

namespace graphene { namespace chain {

void submit_reserve_cycles_to_queue_operation::validate() const
{
  FC_ASSERT( amount > 0, "Must submit a non-zero value" );
  FC_ASSERT( frequency_lock > 0, "Must submit a non-zero value" );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void submit_cycles_to_queue_operation::validate() const
{
  FC_ASSERT( amount > 0 );
  FC_ASSERT( frequency > 0 );
  // Comment's format needs to be x.yy.z
  FC_ASSERT( comment.length() >= 6 && comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void submit_cycles_to_queue_by_license_operation::validate() const
{
  FC_ASSERT( amount > 0, "Must submit a non-zero value" );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void update_queue_parameters_operation::validate() const
{
  if ( reward_interval_time_seconds.valid() )
    FC_ASSERT( *reward_interval_time_seconds > GRAPHENE_MIN_BLOCK_INTERVAL, 
              "Must be greater than the minimal block interval" 
    );
  if ( dascoin_reward_amount.valid() )
    FC_ASSERT( *dascoin_reward_amount > DASCOIN_MIN_DASCOIN_REWARD_AMOUNT,
               "Reward amount too small"
             );
}

void update_global_frequency_operation::validate() const
{
  FC_ASSERT( frequency > 0 );  // TODO: maximum frequency?
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void issue_free_cycles_operation::validate() const
{
  FC_ASSERT( amount > 0 );  // TODO: maximum amount?
  FC_ASSERT( origin < cycle_origin_kind::CYCLE_ORIGIN_KIND_COUNT );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

void issue_cycles_to_license_operation::validate() const
{
  FC_ASSERT( amount > 0 );
  FC_ASSERT( origin.length() <= DASCOIN_MAX_COMMENT_LENGTH );
  FC_ASSERT( comment.length() <= DASCOIN_MAX_COMMENT_LENGTH );
}

} } // namespace graphene::chain
