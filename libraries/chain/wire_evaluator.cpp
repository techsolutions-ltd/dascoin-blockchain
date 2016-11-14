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
    FC_ASSERT( op.asset_to_wire.asset_id == asset_id_type(DASCOIN_WEB_ASSET_INDEX) );

    const auto& acc_obj = op.account(d);
    const auto& asset_obj = op.asset_to_wire.asset_id(d);

    bool insufficient_balance = d.get_balance(op.account, op.asset_to_wire.asset_id).amount >= op.asset_to_wire.amount;
    FC_ASSERT( insufficient_balance,
               "Insufficient Balance: ${balance}, unable to initiate wire transfer '${total}' from account '${a}'",
               ("a", acc_obj.name)
               ("total",d.to_pretty_string(op.asset_to_wire.amount))
               ("balance",d.to_pretty_string(d.get_balance(op.account, op.asset_to_wire.asset_id)))
             );

    asset_dyn_data = &asset_obj.dynamic_asset_data_id(d);
    FC_ASSERT( (asset_dyn_data->current_supply - op.asset_to_wire.amount) >= 0 );

    from_account = &acc_obj;
    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  object_id_type wire_out_evaluator::do_apply(const wire_out_operation& op)
  { try {
    auto& d = db();
    // TODO: make sure that the value comes from cash balance!
    d.adjust_balance( op.account, -op.asset_to_wire.amount );
    return d.create<wire_out_holder_object>([&](wire_out_holder_object& w){
      w.account = op.account;
      w.asset_to_wire = op.asset_to_wire;
    }).id;

  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
