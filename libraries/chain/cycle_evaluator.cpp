/**
 * DASCOIN!
 */

#include <graphene/chain/cycle_evaluator.hpp>
#include <graphene/chain/cycle_objects.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

void_result submit_reserve_cycles_to_queue_evaluator::do_evaluate(const submit_reserve_cycles_to_queue_operation& op)
{ try {
  const auto& d = db();

  // Make sure that we are allowed to issue cycles to accounts:
  FC_ASSERT( d.get_global_properties().parameters.enable_cycle_issuing, "Non-license cycle issuing is disabled" );

  const auto& op_issuer_obj = op.cycle_issuer(d);
  const auto cycle_issuer_id = d.get_chain_authorities().cycle_issuer;

  // Make sure that this is the current license issuer:
  d.perform_chain_authority_check("cycle issuing", cycle_issuer_id, op_issuer_obj);

  // Account must exist:
  const auto& account_obj = op.account(d);

  // Only vault accounts can receive cycles:
  FC_ASSERT( account_obj.is_vault(), "Account '${name}' must be a vault account",
             ("name", account_obj.name)
           );

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type submit_reserve_cycles_to_queue_evaluator::do_apply(const submit_reserve_cycles_to_queue_operation& op)
{ try {
  auto& d = db();
  const auto& params = d.get_global_properties().parameters;

  return d.create<cycle_issue_request_object>([&]( cycle_issue_request_object& req )
  {
    req.cycle_issuer = op.cycle_issuer;
    req.account = op.account;
    req.amount = op.amount;
    req.expiration = d.head_block_time() + fc::seconds(params.cycle_request_expiration_time_seconds);
  }).id;

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result deny_submitting_reserve_cycles_to_queue_evaluator::do_evaluate(const deny_submitting_reserve_cycles_to_queue_operation& op)
{ try {
  const auto& d = db();
  const auto cycle_auth_id = d.get_chain_authorities().cycle_authenticator;
  const auto& op_auth_obj = op.cycle_authenticator(d);

  d.perform_chain_authority_check("cycle authentication", cycle_auth_id, op_auth_obj);

  request_ = &op.request(d);
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type deny_submitting_reserve_cycles_to_queue_evaluator::do_apply(const deny_submitting_reserve_cycles_to_queue_operation& op)
{ try {

  db().remove(*request_);
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result submit_cycles_evaluator::do_evaluate(const submit_cycles_operation& op)
{ try {
  const auto& d = db();
  const auto& account_obj = op.account(d);
  const auto& balance_obj = d.get_cycle_balance_object(op.account);

  // Only vault accounts are allowed to submit cycles:
  FC_ASSERT( account_obj.is_vault(), "Account '${n}' is not a vault account", ("n", account_obj.name) );

  // Assure we have enough funds to submit:
  FC_ASSERT( balance_obj.balance >= op.amount,
             "Cannot submit ${am}, account '${n}' cycle balance is ${b}",
             ("am", op.amount)
             ("n", account_obj.name)
             ("b", balance_obj.balance)
           );

  account_obj_ = &account_obj;
  balance_obj_ = &balance_obj;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type submit_cycles_evaluator::do_apply(const submit_cycles_operation& op)
{ try {
  auto& d = db();

  // Spend cycles, decrease balance and supply:
  d.reserve_cycles(op.account, op.amount);

  // Detrmine the frequency. If the frequency lock is 0 on the license, then the frequency is the current chain
  // frequency:
  frequency_type f = account_obj_->license_info.active_frequency_lock();
  if ( f == 0 )
    f = d.get_dynamic_global_properties().frequency;

  // Create a new element in the reward queue:
  return d.create<reward_queue_object>([&](reward_queue_object& rqo){
    rqo.account = op.account;
    rqo.amount = op.amount;
    rqo.frequency = f;
    rqo.time = d.head_block_time();
  }).id;

} FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
