/**
 * DASCOIN!
 */

#include <graphene/chain/cycle_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/frequency_history_record_object.hpp>
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

  auto origin = fc::reflector<dascoin_origin_kind>::to_string(reserve_cycles);
  return db().push_queue_submission(origin, /*license = */{}, op.account, op.amount, op.frequency_lock, op.comment);

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

  // Make sure that the frequency matches the global frequency:
  const auto GLOB_FREQUENCY = d.get_dynamic_global_properties().frequency;
  FC_ASSERT( op.frequency == GLOB_FREQUENCY,
             "Submit frequency ${op_f} must be equal to global frequency ${glob_f}",
             // This hack should display the frequency with decimals, eg. 2.50 instead of 250.
             ("op_f", asset(op.frequency, d.get_web_asset_id()))
             ("glob_f", asset(GLOB_FREQUENCY, d.get_web_asset_id()))
           );

  _account_obj = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type submit_cycles_to_queue_evaluator::do_apply(const submit_cycles_to_queue_operation& op)
{ try {
  auto& d = db();

  // Spend cycles, decrease balance and supply:
  d.reserve_cycles(op.account, op.amount);
  auto origin = fc::reflector<dascoin_origin_kind>::to_string(user_submit);
  return d.push_queue_submission(origin, /* license = */{}, op.account, op.amount, 
                                 op.frequency, op.comment);

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
    
    // FIXME: This is a hotfix, the previous chain iteration observed dascoin precision of 4 digits
    // The value must be 10 times less to achieve proper precision.
    if ( op.dascoin_reward_amount.valid() )
    {
      gpo.parameters.dascoin_reward_amount = *op.dascoin_reward_amount * 10;
    }
  });

  return _gpo->id;

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_global_frequency_evaluator::do_evaluate(const update_global_frequency_operation& op)
{ try {
  const auto& d = db();
  const auto& gpo = d.get_global_properties();
  const auto& issuer_obj = op.authority(d);

  d.perform_chain_authority_check("license issuer", gpo.authorities.license_issuer, issuer_obj);

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type update_global_frequency_evaluator::do_apply(const update_global_frequency_operation& op)
{ try {
  auto& d = db();
  const auto hb_time = d.head_block_time();

  d.modify(d.get_dynamic_global_properties(), [&op](dynamic_global_property_object& dgpo){
    dgpo.frequency = op.frequency;
  });

  return d.create<frequency_history_record_object>([&op, hb_time](frequency_history_record_object& fhro){
    fhro.authority = op.authority;
    fhro.frequency = op.frequency;
    fhro.time = hb_time;
    fhro.comment = op.comment;
  }).id;

} FC_CAPTURE_AND_RETHROW((op)) }

void_result issue_free_cycles_evaluator::do_evaluate(const issue_free_cycles_operation& op)
{ try {

  const auto& d = db();
  const auto& authority_obj = op.authority(d);
  const auto cycle_issuer_id = d.get_chain_authorities().cycle_issuer;

  // Make sure that this is the current license issuer:
  d.perform_chain_authority_check("cycle issuing", cycle_issuer_id, authority_obj);

  const auto& account_obj = op.account(d);
  const auto& cycle_balance_obj = d.get_cycle_balance_object(op.account);

  FC_ASSERT( account_obj.is_vault(),
             "Account '${name}' must be a vault account",
             ("name", account_obj.name)
           );

  _cycle_balance_obj = &cycle_balance_obj;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result issue_free_cycles_evaluator::do_apply(const issue_free_cycles_operation& op)
{ try {

  db().issue_cycles(*_cycle_balance_obj, op.amount);

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
