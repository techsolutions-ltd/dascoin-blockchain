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

  class wire_out_with_fee_holder_object : public graphene::db::abstract_object<wire_out_with_fee_holder_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_wire_out_with_fee_holder_object_type;

      account_id_type account;
      share_type amount;
      asset_id_type asset_id;
      string currency_of_choice;
      string to_address;
      string memo;
      time_point_sec timestamp;
      extensions_type extensions;

      void set_balance(asset a) { amount = a.amount; asset_id = a.asset_id; }
      void validate() const { };
  };

///////////////////////////////
// MULTI INDEX CONTAINERS:   //
///////////////////////////////

struct by_account_asset;
typedef multi_index_container<
  wire_out_with_fee_holder_object,
  indexed_by<
    ordered_unique< tag<by_id>,
      member<object, object_id_type, &object::id>
    >,
    ordered_non_unique< tag<by_account_asset>,
       composite_key<
          wire_out_with_fee_holder_object,
          member<wire_out_with_fee_holder_object, account_id_type, &wire_out_with_fee_holder_object::account>,
          member<wire_out_with_fee_holder_object, asset_id_type, &wire_out_with_fee_holder_object::asset_id>
       >
    >
  >
> wire_out_with_fee_holder_multi_index_type;

typedef generic_index<wire_out_with_fee_holder_object, wire_out_with_fee_holder_multi_index_type> wire_out_with_fee_holder_index;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::wire_out_with_fee_holder_object, (graphene::db::object),
                    (account)
                    (amount)
                    (asset_id)
                    (currency_of_choice)
                    (to_address)
                    (memo)
                    (timestamp)
                    (extensions)
                  )
