/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type create_license_type_evaluator::do_apply(const create_license_type_operation& op)
{ try {
  using namespace graphene::chain::util;
  auto kind = convert_enum<license_kind>::from_string(op.kind);

  return db().create_license_type(kind,
                                  op.name, 
                                  op.amount, 
                                  op.balance_multipliers, 
                                  op.requeue_multipliers, 
                                  op.return_multipliers,
                                  op.eur_limit);

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result edit_license_type_evaluator::do_evaluate(const edit_license_type_operation& op)
{ try {
  const auto& d = db();
  const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
  const auto& op_admin_obj = op.authority(d);

  d.perform_chain_authority_check("license administration", license_admin_id, op_admin_obj);

  const auto& license_object = op.license_type(d);
  _license_object = &license_object;

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result edit_license_type_evaluator::do_apply(const edit_license_type_operation& op)
{ try {
  auto& d = db();

  d.modify(*_license_object, [op](license_type_object &obj){
     if (op.name.valid())
     {
         obj.name = *op.name;
     }
     if (op.amount.valid())
     {
         obj.amount = *op.amount;
     }
     if (op.eur_limit.valid())
     {
         obj.eur_limit = *op.eur_limit;
     }
  });

  return {};

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

  if ( new_license_obj.kind == license_kind::chartered ||
       new_license_obj.kind == license_kind::promo ||
       new_license_obj.kind == license_kind::locked_frequency )
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

  // If a license already exists, we can only add license of the same kind we had before and we can only improve it:
  if ( account_obj.license_information.valid() )
  {
    const auto& license_information_obj = (*account_obj.license_information)(d);
    const auto& max_license_obj = license_information_obj.max_license(d);

    FC_ASSERT( new_license_obj.kind == license_information_obj.vault_license_kind
                   || (new_license_obj.kind == license_kind::utility2 && license_information_obj.vault_license_kind == license_kind::utility)
                   || (new_license_obj.kind == license_kind::utility && license_information_obj.vault_license_kind == license_kind::utility2),
               "Cannot issue license of kind '${kind}' on account ${a}, current license kind is '${ckind}'",
               ("kind", new_license_obj.kind)
               ("a", account_obj.name)
               ("ckind", fc::reflector<license_kind>::to_string(license_information_obj.vault_license_kind) )
             );
    FC_ASSERT( max_license_obj < new_license_obj,
               "Cannot improve license '${l_max}' on account ${a}, new license '${l_new}' is not an improvement",
               ("a", account_obj.name)
               ("l_max", max_license_obj.name)
               ("l_new", new_license_obj.name)
             );
    
    _license_information_obj = &license_information_obj;
  }

  // Assure that (for chartered licenses) max reward in dascoin would not exceed DASCOIN_MAX_DASCOIN_SUPPLY limit.
  if (new_license_obj.kind == license_kind::chartered)
  {
    share_type license_max_reward_in_dascoin = d.get_licence_max_reward_in_dascoin(new_license_obj, op.bonus_percentage, op.frequency_lock);

    FC_ASSERT(license_max_reward_in_dascoin + d.get_total_dascoin_amount_in_system() <= DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION,
              "Cannot issue license ${l_n} on account ${a}, "
              "because license max reward ${dsc_reward} DSC , "
              "plus amount ${dsc_system} DSC in system, "
              "would exceed DASCOIN_MAX_DASCOIN_SUPPLY limit ${dsc_max_limit} DSC",
              ("l_n", new_license_obj.name)
              ("a", account_obj.name)
              ("dsc_reward", license_max_reward_in_dascoin)
              ("dsc_system", d.get_total_dascoin_amount_in_system())
              ("dsc_max_limit", DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION)
    );
  }

  _issuer_id = issuer_id;
  _account_obj = &account_obj;
  _new_license_obj = &new_license_obj;
  _license_kind = new_license_obj.kind;
  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type issue_license_evaluator::do_apply(const issue_license_operation& op)
{ try {
  auto& d = db();
  auto kind = _new_license_obj->kind;
  share_type amount;

  if ( kind == license_kind::utility )
  {
     amount = _new_license_obj->amount;
     share_type bonus = amount * op.bonus_percentage / 100;
     if(bonus > 0)
     {
        auto origin = fc::reflector<dascoin_origin_kind>::to_string(dascoin_origin_kind::utility_license);
        std::ostringstream comment;
        comment << "Bonus "
                << bonus.value
                << ".";
        d.push_queue_submission(origin, op.license, op.account, bonus, op.frequency_lock, comment.str());
        d.push_applied_operation(
             record_submit_charter_license_cycles_operation(d.get_chain_authorities().license_issuer, op.account, bonus, op.frequency_lock)
        );
     }
  }
  else
  {
     amount = detail::apply_percentage(_new_license_obj->amount, op.bonus_percentage);
  }

  license_information_id_type lic_info_id;

  if ( nullptr == _license_information_obj )
  {
    lic_info_id = d.create<license_information_object>([&](license_information_object& lio){
      lio.account = op.account;
      lio.vault_license_kind = _license_kind;
      lio.add_license(op.license, amount, _new_license_obj->amount, op.bonus_percentage, op.frequency_lock,
                      op.activated_at, d.head_block_time(), _new_license_obj->balance_upgrade,
                      _new_license_obj->up_policy);
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
      lio.add_license(op.license, amount, _new_license_obj->amount, op.bonus_percentage, op.frequency_lock,
                      op.activated_at, d.head_block_time(), _new_license_obj->balance_upgrade,
                      _new_license_obj->up_policy);
      lio.requeue_upgrade += _new_license_obj->requeue_upgrade;
      lio.return_upgrade += _new_license_obj->return_upgrade;
    });
  }

  if ( kind == license_kind::regular || kind == license_kind::locked_frequency || kind == license_kind::utility)
  {
    d.issue_cycles(op.account, amount);
  }
  else if ( kind == license_kind::chartered || kind == license_kind::promo )
  {
    auto origin = fc::reflector<dascoin_origin_kind>::to_string(charter_license);
    d.push_queue_submission(origin, {op.license}, op.account, amount, op.frequency_lock, /* comment = */"");
    // TODO: should we use a virtual op here?
    d.push_applied_operation(
      record_submit_charter_license_cycles_operation(_issuer_id, op.account, amount, op.frequency_lock)
    );
  }

  const auto& dgpo = d.get_dynamic_global_properties();
  const auto limit = d.get_dascoin_limit(*_account_obj, dgpo.last_daily_dascoin_price);
  if (limit.valid())
  {
    d.adjust_balance_limit(*_account_obj, d.get_dascoin_asset_id(), *limit);
  }

  return lic_info_id;

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result update_license_evaluator::do_evaluate(const operation_type& op)
{ try {

  const auto& d = db();
  const auto issuer_id = d.get_chain_authorities().license_issuer;
  const auto op_authority_obj = op.authority(d);

  d.perform_chain_authority_check("license issuing", issuer_id, op_authority_obj);

  const auto& account_obj = op.account(d);
  const auto& license_obj = op.license(d);

  FC_ASSERT( account_obj.is_vault(),
             "Account '${n}' is not a vault account",
             ("n", account_obj.name)
           );

  if ( op.frequency_lock.valid() )
  {
    FC_ASSERT( *(op.frequency_lock) != 0,
               "Cannot update license ${l_n} on account ${a}, frequency lock cannot be zero",
               ("l_n", license_obj.name)
               ("a", account_obj.name)
             );
  }

  FC_ASSERT ( account_obj.license_information.valid(),
              "Cannot update a license on an account which doesn't have an issued license" );

  const auto& license_information_obj = (*account_obj.license_information)(d);

  bool found = false;
  for (uint32_t i = 0; i < license_information_obj.history.size(); ++i)
  {
    if (license_information_obj.history[i].license == op.license)
    {
      found = true;
      _index = i;
      break;
    }
  }

  FC_ASSERT( found,
             "Cannot update license of type '${type}' on account ${a} because that license hasn't been issued",
             ("type", op.license)
             ("a", account_obj.name)
           );

  if ( op.bonus_percentage.valid() )
  {
    FC_ASSERT ( license_information_obj.history[_index].bonus_percent <= *(op.bonus_percentage),
                "Cannot update license of type '${type}' on account ${a} because bonus percentage cannot be decreased (now is ${percentage})",
                ("type", op.license)
                ("a", account_obj.name)
                ("percentage", license_information_obj.history[_index].bonus_percent)
              );
  }

  _license_information_obj = &license_information_obj;

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result update_license_evaluator::do_apply(const operation_type& op)
{ try {
  auto& d = db();

  d.modify(*_license_information_obj, [op, &d, this](license_information_object &obj) {
    if (op.frequency_lock.valid())
      obj.history[_index].frequency_lock = *(op.frequency_lock);
    if (op.bonus_percentage.valid())
    {
      const auto old_amount = detail::apply_percentage(obj.history[_index].base_amount, obj.history[_index].bonus_percent);
      obj.history[_index].bonus_percent = *(op.bonus_percentage);
      const auto new_amount = detail::apply_percentage(obj.history[_index].base_amount, *(op.bonus_percentage));
      obj.history[_index].amount += new_amount - old_amount;
      d.issue_cycles(op.account, new_amount - old_amount);
    }
    if (op.activated_at.valid())
      obj.history[_index].activated_at = *(op.activated_at);
  });

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

} } // namespace graphene::chain
