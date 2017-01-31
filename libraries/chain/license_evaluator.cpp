/**
 * DASCOIN!
 */
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

////////////////////////////
// License type creation: //
////////////////////////////

void_result license_type_create_evaluator::do_evaluate(const license_type_create_operation& op)
{ try {
  const auto& d = db();
  const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
  const auto& op_admin_obj = op.admin(d);

  FC_ASSERT( license_admin_id == op.admin, 
             "Operation must be signed by license administration authority '${la}', signed by '${a}' instead",
             ("la", license_admin_id(d).name)
             ("a", op_admin_obj.name)
           );

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type license_type_create_evaluator::do_apply(const license_type_create_operation& op)
{ try {
  using namespace graphene::chain::util;
  auto kind = convert_enum<license_kind>::from_string(op.kind);

  return db().create_license_type(kind, op.name, op.amount, op.balance_multipliers, op.requeue_multipliers, 
                                  op.return_multipliers);

} FC_CAPTURE_AND_RETHROW( (op) ) }

///////////////////////////
// License type editing: //
///////////////////////////

void_result license_type_edit_evaluator::do_evaluate(const license_type_edit_operation& op)
{ try {
  const auto& d = db();
  const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
  const auto& op_admin_obj = op.admin(d);

  FC_ASSERT( license_admin_id == op.admin,
             "Operation must be signed by license administration authority '${la}', signed by '${a}' instead",
             ("la", license_admin_id(d).name)
             ("a", op_admin_obj.name)
           );

  return {};

}  FC_CAPTURE_AND_RETHROW((op)) }

void_result license_type_edit_evaluator::do_apply(const license_type_edit_operation& op)
{ try {

  db().edit_license_type(op.license, op.name, op.amount, op.balance_multipliers, op.requeue_multipliers, 
                         op.return_multipliers);
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

////////////////////////////
// License type deletion: //
////////////////////////////

void_result license_type_delete_evaluator::do_evaluate(const license_type_delete_operation& op)
{ try {

  // assert_license_authenticator( db(), op.license_authentication_account );
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
  const auto issuer_id = d.get_chain_authorities().license_issuer;
  const auto op_issuer_obj = op.license_issuing_account(d);

  // First, check that the license issuer matches the current license issuing account:
  FC_ASSERT( issuer_id == op.license_issuing_account,
             "Operation must be signed by the license issuing authority '${li}', signed by '${a}' instead",
             ("li", issuer_id(d).name)
             ("a", op_issuer_obj.name)
           );

  const auto& account_obj = op.account(d);
  const auto& new_license_obj = op.license(d);

  // Licenses can only be issued to vault accounts:
  FC_ASSERT( account_obj.is_vault(), "Account '${n}' is not a vault account", ("n", account_obj.name) );

  // Make sure that there is no pending license:
  const auto& info = account_obj.license_info;
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
  const auto auth_id = d.get_chain_authorities().license_authenticator;
  const auto& op_auth_obj = op.license_authentication_account(d);

  FC_ASSERT( auth_id == op.license_authentication_account,
             "Operation must be signed by license authentication account '${la}', signed by '${a}' instead",
             ("la", auth_id(d).name)
             ("a", op_auth_obj.name)
           );

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
