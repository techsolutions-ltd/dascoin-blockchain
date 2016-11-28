/**
 * DASCOIN!
 */

#include <graphene/chain/cycle_evaluator.hpp>
#include <graphene/chain/cycle_objects.hpp>
#include <graphene/chain/database.hpp>


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

  return db().create<cycle_issue_request_object>([&]( cycle_issue_request_object& req )
  {
    req.cycle_issuer = op.cycle_issuer;
    req.account = op.account;
    req.amount = op.amount;
    req.expiration = db().head_block_time() + fc::hours(24);  // TODO: add this to chain parameters.
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

} }  // namespace graphene::chain
