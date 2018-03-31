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

namespace graphene {
namespace chain {

  using graphene::db::abstract_object;

  class issued_asset_record_object : public abstract_object<issued_asset_record_object>
  {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_issued_asset_record_object_type;

      string unique_id;

      account_id_type issuer;
      account_id_type receiver;

      asset_id_type asset_type;
      share_type amount;
      share_type reserved;

      string comment;

      issued_asset_record_object() = default;
      explicit issued_asset_record_object(const string& unique_id, account_id_type issuer,
                                          account_id_type receiver, asset_id_type asset_type,
                                          share_type amount, share_type reserved,
                                          const string& comment)
               : unique_id(unique_id),
                 issuer(issuer),
                 receiver(receiver),
                 asset_type(asset_type),
                 amount(amount),
                 reserved(reserved),
                 comment(comment) {}

      void validate() const;
  };

  struct by_unique_id_asset;
  struct by_receiver_asset;
  typedef multi_index_container<
    issued_asset_record_object,
    indexed_by<
      ordered_unique< 
        tag<by_id>,
        member<object, object_id_type, &object::id> 
      >,
      ordered_unique<
        tag<by_unique_id_asset>,
        composite_key<
          issued_asset_record_object,
          member<issued_asset_record_object, string, &issued_asset_record_object::unique_id>,
          member<issued_asset_record_object, asset_id_type, &issued_asset_record_object::asset_type>
        >
      >,
      ordered_non_unique< 
        tag<by_receiver_asset>,
        composite_key<
          issued_asset_record_object,
          member<issued_asset_record_object, account_id_type, &issued_asset_record_object::receiver>,
          member<issued_asset_record_object, asset_id_type, &issued_asset_record_object::asset_type>
        >
      >
    >
  > issued_asset_record_multi_index_type;

  typedef generic_index<issued_asset_record_object, issued_asset_record_multi_index_type> issued_asset_record_index;

}  // namespace chain
}  // namespace graphene

FC_REFLECT( graphene::chain::issued_asset_record_object,
            (unique_id)
            (issuer)
            (receiver)
            (asset_type)
            (amount)
            (reserved)
            (comment)
          )