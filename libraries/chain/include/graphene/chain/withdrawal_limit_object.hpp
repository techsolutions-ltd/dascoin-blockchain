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
#include <graphene/chain/upgrade_type.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

  class withdrawal_limit_object : public graphene::db::abstract_object<withdrawal_limit_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_withdrawal_limit_object_type;

      account_id_type account;
      asset limit;
      asset spent;
      time_point_sec beginning_of_withdrawal_interval;
      time_point_sec last_withdrawal;
  };

  struct by_account_id;
  using withdrawal_limit_multi_index_type = multi_index_container<
  withdrawal_limit_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member<object, object_id_type, &object::id>
      >,
      ordered_non_unique<
        tag<by_account_id>,
        composite_key< withdrawal_limit_object,
          member< withdrawal_limit_object, account_id_type, &withdrawal_limit_object::account >,
          member< object, object_id_type, &object::id >
        >
      >
    >
  >;
  
  using withdrawal_limit_index = generic_index<withdrawal_limit_object, withdrawal_limit_multi_index_type> ;

} }  // namespace graphene::chain

FC_REFLECT_DERIVED( graphene::chain::withdrawal_limit_object, (graphene::db::object),
                    (account)
                    (limit)
                    (spent)
                    (beginning_of_withdrawal_interval)
                    (last_withdrawal)
                  )
