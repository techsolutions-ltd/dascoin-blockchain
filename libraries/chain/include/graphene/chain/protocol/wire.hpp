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

namespace graphene { namespace chain {

  struct wire_out_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type account;
    asset asset_to_wire;
    string memo;
    extensions_type extensions;

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
    void get_required_owner_authorities( flat_set<account_id_type>& a )const
    {
      a.insert( account );
    }
  };

  struct wire_out_complete_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type wire_out_handler;
    wire_out_holder_id_type holder_object_id;
    extensions_type extensions;

    account_id_type fee_payer() const { return wire_out_handler; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

  struct wire_out_reject_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type wire_out_handler;
    wire_out_holder_id_type holder_object_id;
    extensions_type extensions;

    account_id_type fee_payer() const { return wire_out_handler; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

  struct wire_out_result_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type account;
    bool completed;
    share_type amount;
    asset_id_type asset_id;
    string memo;
    time_point_sec timestamp;

    extensions_type extensions;

    wire_out_result_operation() = default;
    explicit wire_out_result_operation(account_id_type account, bool completed, share_type amount, asset_id_type asset_id,
                                       const string &memo, time_point_sec timestamp)
      : account(account)
      , completed(completed)
      , amount(amount)
      , asset_id(asset_id)
      , memo(memo)
      , timestamp(timestamp) {}

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::wire_out_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_operation,
            (account)
            (asset_to_wire)
            (memo)
            (extensions)
          )

FC_REFLECT( graphene::chain::wire_out_complete_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_complete_operation,
            (wire_out_handler)
            (holder_object_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::wire_out_reject_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_reject_operation,
            (wire_out_handler)
            (holder_object_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::wire_out_result_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_result_operation,
            (account)
            (completed)
            (amount)
            (asset_id)
            (memo)
            (timestamp)
          )
