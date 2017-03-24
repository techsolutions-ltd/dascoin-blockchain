/**
 * DASCOIN!
 */
#include <graphene/chain/license_evaluator.hpp>

#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/account_object.hpp>

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
  const auto op_issuer_obj = op.issuer(d);

  // TODO: refactor this
  d.perform_chain_authority_check("license issuing", issuer_id, op_issuer_obj);

  const auto& account_obj = op.account(d);
  const auto& new_license_obj = op.license(d);

  if ( new_license_obj.kind == license_kind::chartered || new_license_obj.kind == license_kind::promo )
  {
    FC_ASSERT( op.frequency_lock != 0,
               "Cannot issue license ${l_n} on account ${a}, frequency lock cannot be zero",
               ("l_n", new_license_obj.name)
               ("a", account_obj.name)
             );
  }

  FC_ASSERT( account_obj.is_vault(),
             "Account '${n}' is not a vault account",
             ("n", account_obj.name)
           );

  // If a license already exists, we can only improve it:
  if ( account_obj.license_information.valid() )
  {
    const auto& license_information_obj = (*account_obj.license_information)(d);
    const auto& max_license_obj = license_information_obj.max_license(d);

    FC_ASSERT( max_license_obj < new_license_obj,
               "Cannot improve license '${l_max}' on account ${a}, new license '${l_new}' is not an improvement",
               ("a", account_obj.name)
               ("l_max", max_license_obj.name)
               ("l_new", new_license_obj.name)
             );
    
    _license_information_obj = &license_information_obj;
  }

  _issuer_id = issuer_id;
  _account_obj = &account_obj;
  _new_license_obj = &new_license_obj;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type issue_license_evaluator::do_apply(const issue_license_operation& op)
{ try {
  auto& d = db();
  share_type amount = detail::apply_percentage(_new_license_obj->amount, op.bonus_percentage);
  license_information_id_type lic_info_id;

  if ( nullptr == _license_information_obj )
  {
    lic_info_id = d.create<license_information_object>([&](license_information_object& lio){
      lio.account = op.account;
      lio.add_license(op.license, amount, _new_license_obj->amount, op.bonus_percentage, op.frequency_lock, op.activated_at, d.head_block_time());
      lio.balance_upgrade += _new_license_obj->balance_upgrade;
      lio.requeue_upgrade += _new_license_obj->requeue_upgrade;
      lio.return_upgrade += _new_license_obj->return_upgrade;
    }).id;

    d.modify(*_account_obj, [&](account_object& ao){
      ao.license_information = lic_info_id;
    });
  }
  else
  {
    lic_info_id = _license_information_obj->id;

    d.modify(*_license_information_obj, [&](license_information_object& lio){
      lio.add_license(op.license, amount, _new_license_obj->amount, op.bonus_percentage, op.frequency_lock, op.activated_at, d.head_block_time());
      lio.balance_upgrade += _new_license_obj->balance_upgrade;
      lio.requeue_upgrade += _new_license_obj->requeue_upgrade;
      lio.return_upgrade += _new_license_obj->return_upgrade;
    });
  }

  auto kind = _new_license_obj->kind;
  if ( kind == license_kind::regular )
  {
    d.issue_cycles(op.account, amount);
  }
  else if ( kind == license_kind::chartered || kind == license_kind::promo )
  {
    d.create<reward_queue_object>([&](reward_queue_object& rqo){
      rqo.origin = fc::reflector<dascoin_origin_kind>::to_string(charter_license);
      rqo.license = op.license;
      rqo.account = op.account;
      rqo.amount = amount;
      rqo.frequency = op.frequency_lock;
      rqo.time = d.head_block_time();
    });

    // TODO: should we use a virtual op here?
    d.push_applied_operation(
      record_submit_charter_license_cycles_operation(_issuer_id, op.account, amount, op.frequency_lock)
    );
  }

  return lic_info_id;

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
