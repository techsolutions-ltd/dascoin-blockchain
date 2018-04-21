/*
 * MIT License
 *
 * Copyright (c) 2018 TechSolutions Ltd.
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

  struct wire_out_with_fee_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;
    account_id_type account;
    asset asset_to_wire;
    string currency_of_choice;
    string to_address;
    string memo;
    extensions_type extensions;

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct wire_out_with_fee_complete_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;
    account_id_type wire_out_handler;
    wire_out_with_fee_holder_id_type holder_object_id;
    extensions_type extensions;

    account_id_type fee_payer() const { return wire_out_handler; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct wire_out_with_fee_reject_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;
    account_id_type wire_out_handler;
    wire_out_with_fee_holder_id_type holder_object_id;
    extensions_type extensions;

    account_id_type fee_payer() const { return wire_out_handler; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

  struct wire_out_with_fee_result_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;
    account_id_type account;
    bool completed;
    share_type amount;
    asset_id_type asset_id;
    string currency_of_choice;
    string to_address;
    string memo;
    time_point_sec timestamp;
    extensions_type extensions;

    wire_out_with_fee_result_operation() = default;
    explicit wire_out_with_fee_result_operation(account_id_type account, bool completed, share_type amount, asset_id_type asset_id,
                                                const string &currency_of_choice, const string &to_address, const string &memo,
                                                time_point_sec timestamp)
      : account(account)
      , completed(completed)
      , amount(amount)
      , asset_id(asset_id)
      , currency_of_choice(currency_of_choice)
      , to_address(to_address)
      , memo(memo)
      , timestamp(timestamp) {}

    account_id_type fee_payer() const { return account; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type&) const { return 0; }
  };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::wire_out_with_fee_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_with_fee_operation,
            (fee)
            (account)
            (asset_to_wire)
            (currency_of_choice)
            (to_address)
            (memo)
            (extensions)
          )

FC_REFLECT( graphene::chain::wire_out_with_fee_complete_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_with_fee_complete_operation,
            (fee)
            (wire_out_handler)
            (holder_object_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::wire_out_with_fee_reject_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_with_fee_reject_operation,
            (fee)
            (wire_out_handler)
            (holder_object_id)
            (extensions)
          )

FC_REFLECT( graphene::chain::wire_out_with_fee_result_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::wire_out_with_fee_result_operation,
            (fee)
            (account)
            (completed)
            (amount)
            (asset_id)
            (currency_of_choice)
            (to_address)
            (memo)
            (timestamp)
            (extensions)
          )
