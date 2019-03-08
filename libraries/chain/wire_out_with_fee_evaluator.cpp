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

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/wire_out_with_fee_evaluator.hpp>
#include <graphene/chain/withdrawal_limit_object.hpp>

namespace graphene { namespace chain {

  void_result wire_out_with_fee_evaluator::do_evaluate(const wire_out_with_fee_operation& op)
  { try {

    const auto& d = db();

    FC_ASSERT( op.asset_to_wire.asset_id != d.get_dascoin_asset_id(), "Cannot wire out dascoin asset" );
    FC_ASSERT( op.asset_to_wire.asset_id != d.get_cycle_asset_id(), "Cannot wire out cycle asset" );

    const auto& acc_obj = op.account(d);
    const auto& asset_obj = op.asset_to_wire.asset_id(d);
    const auto& dyn_data_obj = asset_obj.dynamic_asset_data_id(d);

    // Check if we have enough balance in the account:
    const auto& from_balance_obj = d.get_balance_object(op.account, op.asset_to_wire.asset_id);
    FC_ASSERT( from_balance_obj.balance >= op.asset_to_wire.amount,
               "Insufficient Balance: ${balance}, unable to initiate wire transfer ${total} from account '${a}'",
               ("a", acc_obj.name)
               ("total", d.to_pretty_string(op.asset_to_wire))
               ("balance", d.to_pretty_string(d.get_balance(op.account, op.asset_to_wire.asset_id)))
             );

    // Check if there is enough asset in the supply to wire out:
    FC_ASSERT( (dyn_data_obj.current_supply - op.asset_to_wire.amount) >= 0,
               "Current supply of ${s} is insufficient to wire out ${a}",
               ("s", d.to_pretty_string(asset(dyn_data_obj.current_supply, op.asset_to_wire.asset_id)))
               ("a", d.to_pretty_string(op.asset_to_wire))
             );

    // Are withdrawal limits in effect?
    if (d.head_block_time() >= HARDFORK_BLC_328_TIME)
    {
      const auto& global_parameters_ext = d.get_global_properties().parameters.extensions;
      auto withdrawal_limit_it = std::find_if(global_parameters_ext.begin(), global_parameters_ext.end(),
                                              [](const chain_parameters::chain_parameters_extension& ext){
                                                   return ext.which() == chain_parameters::chain_parameters_extension::tag< withdrawal_limit_type >::value;
                                            });
      // Is withdrawal limit set?
      if (withdrawal_limit_it != global_parameters_ext.end())
      {
        auto& limit = (*withdrawal_limit_it).get<withdrawal_limit_type>();
        withdrawal_limit_ = &limit;
        // Account is withdrawing an asset which has a limit?
        if (limit.limited_assets.find(op.asset_to_wire.asset_id) != limit.limited_assets.end())
        {
          auto p = d.get_price_in_web_eur(op.asset_to_wire.asset_id);
          FC_ASSERT( p.valid(), "Cannot make a withdrawal because the asset's price is unkown (%{a})", ("a", op.asset_to_wire.asset_id) );
          spent = op.asset_to_wire * *p;
          auto &index = d.get_index_type<withdrawal_limit_index>().indices().get<by_account_id>();
          auto itr = index.find(op.account);
          if (itr != index.end())
          {
            withdrawal_limit_obj_ = &(*itr);
            if (d.head_block_time() - withdrawal_limit_obj_->beginning_of_withdrawal_interval < fc::microseconds(limit.duration * 1000000))
              FC_ASSERT( withdrawal_limit_obj_->limit - withdrawal_limit_obj_->spent >= spent, "Cannot withdraw because of the limit, spent ${s}, amount ${a}", ("s", withdrawal_limit_obj_->spent)("a", spent) );
            else
              FC_ASSERT( withdrawal_limit_obj_->limit >= spent, "Cannot withdraw because of the limit ${l}", ("l", withdrawal_limit_obj_->limit) );
          }
          else
          {
            FC_ASSERT( limit.limit >= op.asset_to_wire, "Withdrawal cannot exceed the absolute limit ${l}, amount ${a}", ("l", limit.limit)("a", spent) );
          }
        }
      }
    }

    from_balance_obj_ = &from_balance_obj;
    asset_dyn_data_ = &dyn_data_obj;
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  object_id_type wire_out_with_fee_evaluator::do_apply(const wire_out_with_fee_operation& op)
  { try {
    auto& d = db();

    if (d.head_block_time() >= HARDFORK_BLC_328_TIME)
    {
      if (!withdrawal_limit_obj_)
      {
        d.create<withdrawal_limit_object>([&](withdrawal_limit_object &o){
          o.account = op.account;
          o.beginning_of_withdrawal_interval = o.last_withdrawal = d.head_block_time();
          o.spent = spent;
          o.limit = withdrawal_limit_->limit;
        });
      }
      else
      {
        d.modify(*withdrawal_limit_obj_, [&](withdrawal_limit_object& o){
          o.last_withdrawal = d.head_block_time();
          if (d.head_block_time() - o.beginning_of_withdrawal_interval > fc::microseconds(withdrawal_limit_->duration * 1000000))
          {
            o.beginning_of_withdrawal_interval = d.head_block_time();
            o.spent = spent;
          } else
          {
            o.spent += spent;
          }
        });
      }
    }

    // Adjust the balance and spent amount:
    d.modify(*from_balance_obj_, [&](account_balance_object& from_b){
      from_b.balance -= op.asset_to_wire.amount;
      from_b.spent += op.asset_to_wire.amount;
    });
    // Contract the supply:
    d.modify(*asset_dyn_data_, [&]( asset_dynamic_data_object& data){
      data.current_supply -= op.asset_to_wire.amount;
    });
    // Create the holder object and return its ID:
    return d.create<wire_out_with_fee_holder_object>([&](wire_out_with_fee_holder_object& wowfho){
      wowfho.account = op.account;
      wowfho.set_balance(op.asset_to_wire);
      wowfho.currency_of_choice = op.currency_of_choice;
      wowfho.to_address = op.to_address;
      wowfho.memo = op.memo;
      wowfho.timestamp = d.head_block_time();
    }).id;

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_with_fee_complete_evaluator::do_evaluate(const wire_out_with_fee_complete_operation& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.wire_out_handler == d.get_chain_authorities().wire_out_handler );

    holder_ = &op.holder_object_id(d);
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_with_fee_complete_evaluator::do_apply(const wire_out_with_fee_complete_operation& op)
  { try {
    db().push_applied_operation(wire_out_with_fee_result_operation{holder_->account, true, holder_->amount, holder_->asset_id,
                                holder_->currency_of_choice, holder_->to_address, holder_->memo, holder_->timestamp});
    // Free the holder object:
    db().remove(*holder_);

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_with_fee_reject_evaluator::do_evaluate(const wire_out_with_fee_reject_operation& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.wire_out_handler == d.get_chain_authorities().wire_out_handler );

    const auto& holder = op.holder_object_id(d);
    const auto& asset_obj = holder.asset_id(d);
    const auto& dyn_data_obj = asset_obj.dynamic_asset_data_id(d);
    const auto& balance_obj = d.get_balance_object(holder.account, holder.asset_id);

    asset_dyn_data_ = &dyn_data_obj;
    balance_obj_ = &balance_obj;
    holder_ = &holder;
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_with_fee_reject_evaluator::do_apply(const wire_out_with_fee_reject_operation& op)
  { try {
    auto& d = db();
    // Revert to the before state: increase the balance amount.
    d.modify(*balance_obj_, [&](account_balance_object& b){
     b.balance += holder_->amount;
     // TODO: The spending limit should not be restored, it may become negative!
    });
    // Expand the supply:
    d.modify(*asset_dyn_data_, [&]( asset_dynamic_data_object& data){
      data.current_supply += holder_->amount;
    });
    db().push_applied_operation(wire_out_with_fee_result_operation{holder_->account, false, holder_->amount, holder_->asset_id,
                                                          holder_->currency_of_choice, holder_->to_address, holder_->memo, holder_->timestamp});
    // Free the holder object:
    d.remove(*holder_);

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
