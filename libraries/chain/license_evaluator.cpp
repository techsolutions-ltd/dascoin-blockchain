/**
 * DASCOIN!
 */
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

//////////////////////
// Private methods: //
//////////////////////

void assert_license_authenticator(const database& db, account_id_type account)
{
	FC_ASSERT( account == db.get_global_properties().authorities.license_authenticator );
}

void assert_license_issuer(const database& db, account_id_type account)
{
  FC_ASSERT( account == db.get_global_properties().authorities.license_issuer );
}

////////////////////////////
// License type creation: //
////////////////////////////

void_result license_type_create_evaluator::do_evaluate(const license_type_create_operation& op)
{ try {

  assert_license_authenticator(db(), op.license_authentication_account);
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type license_type_create_evaluator::do_apply(const license_type_create_operation& op)
{ try {

  return db().create_license_type(op.name, op.amount, op.policy);

} FC_CAPTURE_AND_RETHROW( (op) ) }

///////////////////////////
// License type editing: //
///////////////////////////

void_result license_type_edit_evaluator::do_evaluate(const license_type_edit_operation& op)
{ try {

  assert_license_authenticator(db(), op.license_authentication_account);
  return {};

}  FC_CAPTURE_AND_RETHROW( (op)) }

void_result license_type_edit_evaluator::do_apply(const license_type_edit_operation& op)
{ try {

  db().modify( db().get(op.license), [&]( license_type_object& lic ) {
    if (op.name.valid()) lic.name = *op.name;
    if (op.amount.valid()) lic.amount = *op.amount;
    if (op.policy.valid()) lic.policy = *op.policy;
  });
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

////////////////////////////
// License type deletion: //
////////////////////////////

void_result license_type_delete_evaluator::do_evaluate(const license_type_delete_operation& op)
{ try {

  assert_license_authenticator( db(), op.license_authentication_account );
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_type_delete_evaluator::do_apply(const license_type_delete_operation& op)
{ try {

  // TODO: to do a meaningfull delete, all accounts with a license must have the license_id removed!
  return void_result();

} FC_CAPTURE_AND_RETHROW( (op) ) }

////////////////////////////
// License issue request: //
////////////////////////////

void_result license_request_evaluator::do_evaluate(const license_request_operation& op)
{ try {

  const auto& d = db();
  const auto& account_obj = op.account(d);
  const auto& new_license_obj = op.license(d);

  // First, check that the license issuer matches the current license issuing account:
  assert_license_issuer( d, op.license_issuing_account );

  // Licenses can only be issued to vault accounts:
  FC_ASSERT( account_obj.is_vault(), "Account '${n}' is not a vault account", ("n", account_obj.name) );

  // If the account has an active license, then we need to check if we can IMPROVE it:
  const auto active_lic_opt = account_obj.license_info.active_license();
  if ( active_lic_opt.valid() )
  {
    const auto& active_license_obj = (*active_lic_opt)(d);
    FC_ASSERT( active_license_obj < new_license_obj,
               "Cannot improve license on account ${a}, license ${l_old} is better than the active license ${l_act}",
               ("a", account_obj.name)
               ("l_old", active_license_obj.name)
               ("l_new", new_license_obj.name)
             );
  }

  // If there is a license request pending, check if the new license is better and replace it with a new license:
  const auto pending_lic_opt = account_obj.license_info.pending_license;
  if ( pending_lic_opt.valid())
  {
    const auto& pending_license_object = (*pending_lic_opt)(d);
    FC_ASSERT( pending_license_object < new_license_obj,
               "Cannot issue request for license '${nln}' on account '${a}', pending license '${pln}' is better",
               ("a", account_obj.name)
               ("pln", pending_license_object.name)
               ("nln", new_license_obj.name)
             );
  }

  account_obj_ = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type license_request_evaluator::do_apply(const license_request_operation& op)
{ try {
  auto& d = db();
  const auto& params = d.get_global_properties().parameters;

  // Update the pending license:
  d.modify(*account_obj_, [&](account_object& a){
    a.license_info.pending_license = op.license;
  });

  // Create the new request object:
  return d.create<license_request_object>([&](license_request_object &req) {
    req.license_issuing_account = op.license_issuing_account;
    req.account = op.account;
    req.license = op.license;
    req.frequency = op.frequency;
    req.expiration = d.head_block_time() + fc::seconds(params.license_expiration_time_seconds);
  }).id;

  ilog("Pending request for ${n}", ("n", account_obj_->license_info.pending_license));

} FC_CAPTURE_AND_RETHROW( (op) ) }

/////////////////////////////
// License request denial: //
/////////////////////////////

void_result license_deny_evaluator::do_evaluate(const license_deny_operation& op)
{ try {

  const auto& _db = db();

  assert_license_authenticator( _db, op.license_authentication_account );
  request_obj_ = &op.request(_db);
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_deny_evaluator::do_apply(const license_deny_operation& op)
{ try {

  // TODO: if additional processing is required, do it in database::deny_license_request().
  db().remove(*request_obj_);
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
