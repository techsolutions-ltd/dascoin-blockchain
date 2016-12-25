/**
 * DASCOIN!
 */

#include <graphene/chain/cycle_evaluator.hpp>
#include <graphene/chain/cycle_objects.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/queue_objects.hpp>

namespace graphene { namespace chain {

void_result cycle_issue_request_evaluator::do_evaluate(const cycle_issue_request_operation& op)
{ try {
  const auto& d = db();

  // Make sure that we are allowed to issue cycles to accounts:
  FC_ASSERT( d.get_global_properties().parameters.enable_cycle_issuing, "Non-license cycle issuing is disabled" );

  // Make sure that this is the current license issuer:
  FC_ASSERT( op.cycle_issuer == d.get_chain_authorities().cycle_issuer,
             "Issue request operation improperly signed by issuer '${op_issuer}'; current cycle issuer is '${chain_issuer}'",
             ("op_issuer", op.cycle_issuer(d).name)
             ("chain_issuer", d.get_chain_authorities().cycle_issuer(d).name)
           );

  // Account must exist:
  const auto& account_obj = op.account(d);

  // Only vault accounts can receive cycles:
  FC_ASSERT( account_obj.is_vault(), "Account '${name}' must be a vault account",
             ("name", account_obj.name)
           );

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type cycle_issue_request_evaluator::do_apply(const cycle_issue_request_operation& op)
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

void_result cycle_issue_deny_evaluator::do_evaluate(const cycle_issue_deny_operation& op)
{ try {
  const auto& d = db();

  // Make sure that this is the current license issuer:
  FC_ASSERT( op.cycle_authenticator == d.get_chain_authorities().cycle_authenticator,
             "Issue deny request operation improperly signed by authenticator '${op_a}'; current cycle authenticator is '${chain_a}'",
             ("op_a", op.cycle_authenticator(d).name)
             ("chain_a", d.get_chain_authorities().cycle_authenticator(d).name)
           );

  request_ = &op.request(d);
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type cycle_issue_deny_evaluator::do_apply(const cycle_issue_deny_operation& op)
{ try {

  db().remove(*request_);
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result submit_cycles_evaluator::do_evaluate(const submit_cycles_operation& op)
{ try {
  const auto& d = db();
  const auto& account_obj = op.account(d);
  const auto& balance_obj = d.get_cycle_balance_object(op.account);

  // Only wallet accounts are allowed to submit cycles:
  FC_ASSERT( account_obj.is_wallet(),
             "Account '${n}' is not a wallet account",
             ("n", account_obj.name)
           );

  // Assure we have enough funds to submit:
  FC_ASSERT( balance_obj.balance > op.amount,
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

  // Decrease the cycle balance:
  d.modify(*balance_obj_, [&](account_cycle_balance_object& acbo){
    acbo.balance -= op.amount;
  });

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
