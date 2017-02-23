/**
 * DASCOIN!
 */
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

////////////////////////////
// Private methods:       //
////////////////////////////

namespace detail {

share_type apply_percentage(share_type val, share_type percent)
{
  return val + (val * percent / 100);
};

}  // namespace graphene::chain::detail

////////////////////////////
// License type creation: //
////////////////////////////

void_result create_license_type_evaluator::do_evaluate(const create_license_type_operation& op)
{ try {
  const auto& d = db();
  const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
  const auto& op_admin_obj = op.admin(d);

  d.perform_chain_authority_check("license administration", license_admin_id, op_admin_obj);

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type create_license_type_evaluator::do_apply(const create_license_type_operation& op)
{ try {
  using namespace graphene::chain::util;
  auto kind = convert_enum<license_kind>::from_string(op.kind);

  return db().create_license_type(kind, op.name, op.amount, op.balance_multipliers, op.requeue_multipliers, 
                                  op.return_multipliers);

} FC_CAPTURE_AND_RETHROW( (op) ) }

////////////////////////////
// License issue request: //
////////////////////////////

void_result issue_license_evaluator::do_evaluate(const issue_license_operation& op)
{ try {

  const auto& d = db();
  const auto issuer_id = d.get_chain_authorities().license_issuer;
  const auto op_issuer_obj = op.license_issuer(d);

  // First, check that the license issuer matches the current license issuing account:
  d.perform_chain_authority_check("license issuing", issuer_id, op_issuer_obj);

  const auto& account_obj = op.account(d);
  const auto& new_license_obj = op.license(d);

  // For charter licenses: frequency lock cannot be zero:
  if ( new_license_obj.kind == license_kind::chartered || new_license_obj.kind == license_kind::promo )
  {
    FC_ASSERT( op.frequency != 0,
               "Cannot issue license ${l_n} on account ${a}, frequency lock cannot be zero",
               ("l_n", new_license_obj.name)
               ("a", account_obj.name)
             );
  }

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

  new_license_obj_ = &new_license_obj;
  account_obj_ = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type issue_license_evaluator::do_apply(const issue_license_operation& op)
{ try {
  auto& d = db();
  const auto& params = d.get_global_properties().parameters;

  share_type amount = detail::apply_percentage(new_license_obj_->amount, op.bonus_percentage);

  // Create the new request object:
  license_request_id_type req_id =  d.create<license_request_object>([&](license_request_object &req) {
    req.license_issuer = op.license_issuer;
    req.account = op.account;
    req.license = op.license;
    req.amount = amount;
    req.frequency_lock = op.frequency;
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

void_result deny_license_evaluator::do_evaluate(const deny_license_operation& op)
{ try {
  const auto& d = db();
  const auto auth_id = d.get_chain_authorities().license_authenticator;
  const auto& op_auth_obj = op.license_authenticator(d);

  d.perform_chain_authority_check("license authentication", auth_id, op_auth_obj);

  const auto& request_obj = op.request(d);
  const auto& account_obj = request_obj.account(d);
  const auto& info = account_obj.license_info;

  FC_ASSERT( (*info.pending).request == request_obj.id );
  FC_ASSERT( (*info.pending).license == request_obj.license );

  request_obj_ = &request_obj;
  account_obj_ = &account_obj;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result deny_license_evaluator::do_apply(const deny_license_operation& op)
{ try {
  auto& d = db();

  d.modify(*account_obj_, [&](account_object& ao){
    ao.license_info.clear_pending();
  });
  d.remove(*request_obj_);

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
