/**
 * DASCOIN!
 */
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/wire_evaluator.hpp>

namespace graphene { namespace chain {

  void_result wire_out_evaluator::do_evaluate(const wire_out_operation& op)
  { try {

    const auto& d = db();
    // We can only wire out web assets for now. TODO: assets must be marked for wire out ability.
    FC_ASSERT( op.asset_to_wire.asset_id == d.get_web_asset_id() );

    const auto& acc_obj = op.account(d);
    const auto& asset_obj = op.asset_to_wire.asset_id(d);
    const auto& dyn_data_obj = asset_obj.dynamic_asset_data_id(d);

    // Get the limit:
    share_type wire_limit = acc_obj.get_max_from_limit(limit_kind::wallet_out_webasset);

    // Check if we have enough balance in the account:
    const auto& from_balance_obj = d.get_balance_object(op.account, d.get_web_asset_id());
    FC_ASSERT( from_balance_obj.balance >= op.asset_to_wire.amount,
               "Insufficient Balance: ${balance}, unable to initiate wire transfer ${total} from account '${a}'",
               ("a", acc_obj.name)
               ("total",d.to_pretty_string(op.asset_to_wire))
               ("balance",d.to_pretty_string(d.get_balance(op.account, op.asset_to_wire.asset_id)))
             );

    // Check if there is enough asset in the supply to wire out:
    FC_ASSERT( (dyn_data_obj.current_supply - op.asset_to_wire.amount) >= 0,
               "Current supply of ${s} is insufficient to wire out ${a}",
               ("s", d.to_pretty_string(asset(dyn_data_obj.current_supply, d.get_web_asset_id())))
               ("a", d.to_pretty_string(op.asset_to_wire))
             );

    // Check if the wire out would breach the limit:
    FC_ASSERT( from_balance_obj.spent < wire_limit,
               "Wallet limit has been exceeded, ${spent}/${max} on account ${a}",
               ("a", acc_obj.name)
               ("spent", d.to_pretty_string(asset(from_balance_obj.spent, d.get_web_asset_id())))
               ("max", d.to_pretty_string(asset(wire_limit, d.get_web_asset_id())))
             );

    // from_account_ = &acc_obj;
    from_balance_obj_ = &from_balance_obj;
    asset_dyn_data_ = &dyn_data_obj;
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  object_id_type wire_out_evaluator::do_apply(const wire_out_operation& op)
  { try {
    auto& d = db();
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
    return d.create<wire_out_holder_object>([&](wire_out_holder_object& w){
      w.account = op.account;
      w.set_balance(op.asset_to_wire);
      w.memo = op.memo;
    }).id;

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_complete_evaluator::do_evaluate(const wire_out_complete_operation& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.wire_out_handler == d.get_chain_authorities().wire_out_handler );

    holder_ = &op.holder_object_id(d);
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_complete_evaluator::do_apply(const wire_out_complete_operation& op)
  { try {
    // Free the holder object:
    db().remove(*holder_);
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_reject_evaluator::do_evaluate(const wire_out_reject_operation& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.wire_out_handler == d.get_chain_authorities().wire_out_handler );

    const auto& holder = op.holder_object_id(d);
    const auto& asset_obj = holder.asset_id(d);
    const auto& dyn_data_obj = asset_obj.dynamic_asset_data_id(d);
    const auto& balance_obj = d.get_balance_object(holder.account, d.get_web_asset_id());

    asset_dyn_data_ = &dyn_data_obj;
    balance_obj_ = &balance_obj;
    holder_ = &holder;
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result wire_out_reject_evaluator::do_apply(const wire_out_reject_operation& op)
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
    // Free the holder object:
    d.remove(*holder_);
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
