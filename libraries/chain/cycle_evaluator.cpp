/**
 * DASCOIN!
 */

#include <graphene/chain/cycle_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

void_result submit_reserve_cycles_to_queue_evaluator::do_evaluate(const submit_reserve_cycles_to_queue_operation& op)
{ try {
  const auto& d = db();

  FC_ASSERT( d.get_global_properties().parameters.enable_cycle_issuing,
             "Submitting reserve cycles to the queue is disabled"
           );

  const auto& op_issuer_obj = op.issuer(d);
  const auto cycle_issuer_id = d.get_chain_authorities().cycle_issuer;

  // Make sure that this is the current license issuer:
  d.perform_chain_authority_check("cycle issuing", cycle_issuer_id, op_issuer_obj);

  const auto& account_obj = op.account(d);

  FC_ASSERT( account_obj.is_vault(),
             "Account '${name}' must be a vault account",
             ("name", account_obj.name)
           );

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type submit_reserve_cycles_to_queue_evaluator::do_apply(const submit_reserve_cycles_to_queue_operation& op)
{ try {
  auto& d = db();

  return d.create<reward_queue_object>([&](reward_queue_object& rqo){
    rqo.origin = fc::reflector<dascoin_origin_kind>::to_string(reserve_cycles);
    rqo.license = {};
    rqo.account = op.account;
    rqo.amount = op.amount;
    rqo.frequency = op.frequency_lock;
    rqo.time = d.head_block_time();
  }).id;

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result submit_cycles_to_queue_evaluator::do_evaluate(const submit_cycles_to_queue_operation& op)
{ try {
  const auto& d = db();
  const auto& account_obj = op.account(d);
  const auto& balance_obj = d.get_cycle_balance_object(op.account);

  // Only vault accounts are allowed to submit cycles:
  FC_ASSERT( account_obj.is_vault(),
             "Account '${n}' is not a vault account",
             ("n", account_obj.name)
           );

  // Assure we have enough funds to submit:
  FC_ASSERT( balance_obj.balance >= op.amount,
             "Cannot submit ${am}, account '${n}' cycle balance is ${b}",
             ("am", op.amount)
             ("n", account_obj.name)
             ("b", balance_obj.balance)
           );

  _account_obj = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type submit_cycles_to_queue_evaluator::do_apply(const submit_cycles_to_queue_operation& op)
{ try {
  auto& d = db();

  // Spend cycles, decrease balance and supply:
  d.reserve_cycles(op.account, op.amount);

  frequency_type frequency = 0;
  if( _account_obj->license_information.valid() )
    frequency = (*_account_obj->license_information)(d).frequency_lock;
  if ( 0 == frequency )
    frequency = d.get_dynamic_global_properties().frequency;

  return db().create<reward_queue_object>([&](reward_queue_object& rqo){
    rqo.origin = fc::reflector<dascoin_origin_kind>::to_string(user_submit);
    rqo.license = {};
    rqo.account = op.account;
    rqo.amount = op.amount;
    rqo.frequency = frequency;
    rqo.time = d.head_block_time();
  }).id;

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_queue_parameters_evaluator::do_evaluate(const update_queue_parameters_operation& op)
{ try {
  const auto& d = db();
  const auto& gpo = d.get_global_properties();
  const auto& issuer_obj = op.issuer(d);

  d.perform_chain_authority_check("license issuer", gpo.authorities.license_issuer, issuer_obj);

  if ( op.reward_interval_time_seconds.valid()  )
    FC_ASSERT( *op.reward_interval_time_seconds % gpo.parameters.block_interval == 0,
               "Reward interval must be a multiple of the block interval ${bi}",
               ("bi", gpo.parameters.block_interval)
             );

  _gpo = &gpo;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type update_queue_parameters_evaluator::do_apply(const update_queue_parameters_operation& op)
{ try {
  auto& d = db();

  d.modify(*_gpo, [&](global_property_object& gpo){
    CHECK_AND_SET_OPT(gpo.parameters.enable_dascoin_queue, op.enable_dascoin_queue);
    CHECK_AND_SET_OPT(gpo.parameters.reward_interval_time_seconds, op.reward_interval_time_seconds);
    CHECK_AND_SET_OPT(gpo.parameters.dascoin_reward_amount, op.dascoin_reward_amount);
  });

  return _gpo->id;

} FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
