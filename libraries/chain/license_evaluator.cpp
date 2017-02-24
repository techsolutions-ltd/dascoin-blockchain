/**
 * DASCOIN!
 */
#include <graphene/chain/license_evaluator.hpp>

namespace graphene { namespace chain {

namespace detail {

share_type apply_percentage(share_type val, share_type percent)
{
  return val + (val * percent / 100);
};

}  // namespace graphene::chain::detail

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

  return db().create_license_type(kind,
                                  op.name, 
                                  op.amount, 
                                  op.balance_multipliers, 
                                  op.requeue_multipliers, 
                                  op.return_multipliers);

} FC_CAPTURE_AND_RETHROW( (op) ) }

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

  // If the account has an active license, then we need to check if we can IMPROVE it:
  const auto active_lic_opt = account_obj.license_info.max_license();
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

void_result issue_license_evaluator::do_apply(const issue_license_operation& op)
{ try {
  auto& d = db();
  share_type amount = detail::apply_percentage(new_license_obj_->amount, op.bonus_percentage);

  d.modify(*account_obj_, [&](account_object& a) {
    auto& info = a.license_info;

    // Add the license to the top of the history, so it becomes the new active license:
    info.add_license(op.license, amount, op.frequency);

    // Improve all the upgrades to match the new license:
    info.balance_upgrade += new_license_obj_->balance_upgrade;
    info.requeue_upgrade += new_license_obj_->requeue_upgrade;
    info.return_upgrade += new_license_obj_->return_upgrade;
  });

  auto kind = new_license_obj_->kind;
  if ( kind == license_kind::regular )
    d.issue_cycles(account_obj_->id, amount);
  else if ( kind == license_kind::chartered || kind == license_kind::promo )
    d.submit_cycles_to_queue(account_obj_->id, amount, op.frequency);

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
