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
  using namespace graphene::chain::util;
  auto& d = db();
  auto kind = convert_enum<license_kind>::from_string(op.kind);

  return d.create<license_type_object>([&](license_type_object& lto){
    lto.name = op.name;
    lto.amount = op.amount;
    lto.kind = kind;
    // TODO: set the upgrades.
  }).id;

  // Deprecated, no longer using dynamic policy of license creation.
  // return db().create_license_type(op.name, op.amount, op.policy);

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
  const auto& info = account_obj.license_info;

  // First, check that the license issuer matches the current license issuing account:
  assert_license_issuer(d, op.license_issuing_account);

  // Licenses can only be issued to vault accounts:
  FC_ASSERT( account_obj.is_vault(), "Account '${n}' is not a vault account", ("n", account_obj.name) );

  // Make sure that there is no pending license:
  FC_ASSERT( !info.pending.valid(),
             "Cannot issue license ${l_n} on account ${a}, license ${l_p} is pending",
             ("l_n", new_license_obj.name)
             ("a", account_obj.name)
             ("l_p", (*info.pending).license(d).name)
           );

  // If the account has an active license, then we need to check if we can IMPROVE it:
  const auto active_lic_opt = info.active_license();
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

  account_obj_ = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type license_request_evaluator::do_apply(const license_request_operation& op)
{ try {
  auto& d = db();
  const auto& params = d.get_global_properties().parameters;

  // Create the new request object:
  license_request_id_type req_id =  d.create<license_request_object>([&](license_request_object &req) {
    req.license_issuing_account = op.license_issuing_account;
    req.account = op.account;
    req.license = op.license;
    req.frequency = op.frequency;
    req.expiration = d.head_block_time() + fc::seconds(params.license_expiration_time_seconds);
  }).id;

  // Update the pending license:
  d.modify(*account_obj_, [&](account_object& a){
    a.license_info.set_pending(op.license, req_id);
  });

  return req_id;

} FC_CAPTURE_AND_RETHROW( (op) ) }

/////////////////////////////
// License request denial: //
/////////////////////////////

void_result license_deny_evaluator::do_evaluate(const license_deny_operation& op)
{ try {
  const auto& d = db();

  assert_license_authenticator(d, op.license_authentication_account);

  const auto& request_obj = op.request(d);
  const auto& account_obj = request_obj.account(d);
  const auto& info = account_obj.license_info;

  FC_ASSERT( (*info.pending).request == request_obj.id );
  FC_ASSERT( (*info.pending).license == request_obj.license );

  request_obj_ = &request_obj;
  account_obj_ = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result license_deny_evaluator::do_apply(const license_deny_operation& op)
{ try {
  auto& d = db();

  d.modify(*account_obj_, [&](account_object& ao){
    ao.license_info.clear_pending();
  });
  d.remove(*request_obj_);

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
