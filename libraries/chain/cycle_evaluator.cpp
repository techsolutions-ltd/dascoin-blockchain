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

#include <graphene/chain/cycle_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/frequency_history_record_object.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/submit_cycles_evaluator_helper.hpp>
#include <graphene/db/object_id.hpp>
#include <graphene/chain/access_layer.hpp>

namespace graphene { namespace chain {

void_result submit_reserve_cycles_to_queue_evaluator::do_evaluate(const submit_reserve_cycles_to_queue_operation& op)
{ try {
  const auto& d = db();

  FC_ASSERT( d.get_global_properties().parameters.enable_cycle_issuing,
             "Submitting reserve cycles to the queue is disabled"
           );

  const auto& op_issuer_obj = op.issuer(d);
  const auto cycle_issuer_id = d.get_chain_authorities().cycle_issuer;

  // Make sure that this is the current license issuer:
  d.perform_chain_authority_check("cycle issuing", cycle_issuer_id, op_issuer_obj);

  const auto& account_obj = op.account(d);

  FC_ASSERT( account_obj.is_vault(),
             "Account '${name}' must be a vault account",
             ("name", account_obj.name)
           );

  // Assure that amount of cycles submitted would not exceed DASCOIN_MAX_DASCOIN_SUPPLY limit.
  FC_ASSERT(d.cycles_to_dascoin(op.amount, op.frequency_lock) + d.get_total_dascoin_amount_in_system() <= DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION,
            "Cannot submit ${am} cycles with frequency (${f}), "
            "because with amount (${dsc_system} DSC in system, "
            "it would exceed DASCOIN_MAX_DASCOIN_SUPPLY limit ${dsc_max_limit} DSC",
            ("am", op.amount)
            ("f", op.frequency_lock)
            ("dsc_system", d.get_total_dascoin_amount_in_system())
            ("dsc_max_limit", DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION)
          );

  return {};

} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type submit_reserve_cycles_to_queue_evaluator::do_apply(const submit_reserve_cycles_to_queue_operation& op)
{ try {

  auto origin = fc::reflector<dascoin_origin_kind>::to_string(reserve_cycles);
  return db().push_queue_submission(origin, /*license = */{}, op.account, op.amount, op.frequency_lock, op.comment);

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result submit_cycles_to_queue_evaluator::do_evaluate(const submit_cycles_to_queue_operation& op)
{ try {
  detail::submit_cycles_evaluator_helper helper(db());
  fc::from_variant<license_type_object::space_id, license_type_object::type_id>(variant{op.comment}, _license_type);
  _license_information_obj = helper.do_evaluate(op, _license_type, op.frequency);

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type submit_cycles_to_queue_evaluator::do_apply(const submit_cycles_to_queue_operation& op)
{ try {
  detail::submit_cycles_evaluator_helper helper(db());
  return helper.do_apply(op, _license_information_obj, _license_type, op.frequency);

} FC_CAPTURE_AND_RETHROW((op)) }

void_result fee_pool_cycles_submit_evaluator::do_evaluate(const fee_pool_cycles_submit_operation& op)
{ try {
   auto& d = db();
   const auto& account_obj = op.issuer(d);

     // Only fee pool account is allowed to submit cycles with this operation
     FC_ASSERT( op.issuer == d.get_dynamic_global_properties().fee_pool_account_id,
                "Issuer '${n}' is not a fee pool account",
                ("n", account_obj.name)
              );

     auto& cycle_balance = d.get_balance_object(op.issuer, d.get_cycle_asset_id());

     // Assure we have enough funds to submit:
     FC_ASSERT( cycle_balance.balance >= op.amount,
                "Cannot submit ${am} cycles, account '${n}' fee pool account cycle balance is ${b}",
                ("am", op.amount)
                ("n", account_obj.name)
                ("b", cycle_balance.balance)
              );

     // Assure that amount of cycles submitted would not exceed DASCOIN_MAX_DASCOIN_SUPPLY limit.
     FC_ASSERT(d.cycles_to_dascoin(op.amount, d.get_dynamic_global_properties().frequency) + d.get_total_dascoin_amount_in_system() <= DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION,
               "Cannot submit ${am} cycles with frequency (${f}), "
               "because with amount (${dsc_system} DSC in system, "
               "it would exceed DASCOIN_MAX_DASCOIN_SUPPLY limit ${dsc_max_limit} DSC",
               ("am", op.amount)
               ("f", d.get_dynamic_global_properties().frequency)
               ("dsc_system", d.get_total_dascoin_amount_in_system())
               ("dsc_max_limit", DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION)
             );

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type fee_pool_cycles_submit_evaluator::do_apply(const fee_pool_cycles_submit_operation& op)
{ try {

   auto& d = db();
   auto origin = fc::reflector<dascoin_origin_kind>::to_string(user_submit);

   auto& cycle_balance = d.get_balance_object(op.issuer, d.get_cycle_asset_id());
   d.modify(cycle_balance, [&op](account_balance_object& balance_obj){
      balance_obj.balance -= op.amount;
   });

   d.modify(d.get_cycle_asset().dynamic_asset_data_id(d), [&](asset_dynamic_data_object& addo)
   {
      addo.current_supply -= op.amount;
   });

   return d.push_queue_submission(origin, optional<license_type_id_type>(), op.issuer, op.amount, d.get_dynamic_global_properties().frequency, op.comment);

} FC_CAPTURE_AND_RETHROW((op)) }

void_result submit_cycles_to_queue_by_license_evaluator::do_evaluate(const operation_type& op)
{ try {
  detail::submit_cycles_evaluator_helper helper(db());
  _license_information_obj = helper.do_evaluate(op, op.license_type, op.frequency_lock);

  const auto& d = db();
  database_access_layer _dal(d);
  const auto& lic = _dal.get_license_type(op.license_type);

  //TODO: Write helper function for code below:
  if (lic.valid()) {
    auto is_manual_submit_license =
        lic->kind == license_kind::locked_frequency ||
        lic->kind == license_kind::utility ||
        lic->kind == license_kind::package;
    FC_ASSERT(is_manual_submit_license, "You can only submit cycles from manual-submit license kind." );
  }

  //FC_ASSERT( _license_information_obj->vault_license_kind != chartered, "Cannot submit cycles to the minting queue from a chartered license" );
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type submit_cycles_to_queue_by_license_evaluator::do_apply(const operation_type& op)
{ try {
  detail::submit_cycles_evaluator_helper helper(db());
  return helper.do_apply(op, _license_information_obj, op.license_type, op.frequency_lock);

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_queue_parameters_evaluator::do_evaluate(const update_queue_parameters_operation& op)
{ try {
  const auto& d = db();
  const auto& gpo = d.get_global_properties();
  const auto& issuer_obj = op.issuer(d);

  d.perform_chain_authority_check("license issuer", gpo.authorities.license_issuer, issuer_obj);

  if ( op.reward_interval_time_seconds.valid() )
    FC_ASSERT( *op.reward_interval_time_seconds % gpo.parameters.block_interval == 0,
               "Reward interval must be a multiple of the block interval ${bi}",
               ("bi", gpo.parameters.block_interval)
             );

  _gpo = &gpo;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type update_queue_parameters_evaluator::do_apply(const update_queue_parameters_operation& op)
{ try {
  auto& d = db();

  d.modify(*_gpo, [&](global_property_object& gpo){
    CHECK_AND_SET_OPT(gpo.parameters.enable_dascoin_queue, op.enable_dascoin_queue);
    CHECK_AND_SET_OPT(gpo.parameters.reward_interval_time_seconds, op.reward_interval_time_seconds);
    
    if ( op.dascoin_reward_amount.valid() )
    {
      gpo.parameters.dascoin_reward_amount = *op.dascoin_reward_amount;
      if( d.head_block_time() < HARDFORK_BLC_58_TIME )
      {
        // Before BLC_58 hardfork we need to multiply the amount by 10 (because we were using 4 decimal places):
        gpo.parameters.dascoin_reward_amount *= 10;
      }
    }
  });

  return _gpo->id;

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_global_frequency_evaluator::do_evaluate(const update_global_frequency_operation& op)
{ try {
  const auto& d = db();
  const auto& gpo = d.get_global_properties();
  const auto& issuer_obj = op.authority(d);

  d.perform_chain_authority_check("license issuer", gpo.authorities.license_issuer, issuer_obj);

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type update_global_frequency_evaluator::do_apply(const update_global_frequency_operation& op)
{ try {
  auto& d = db();
  const auto hb_time = d.head_block_time();

  d.modify(d.get_dynamic_global_properties(), [&op](dynamic_global_property_object& dgpo){
    dgpo.frequency = op.frequency;
  });

  return d.create<frequency_history_record_object>([&op, hb_time](frequency_history_record_object& fhro){
    fhro.authority = op.authority;
    fhro.frequency = op.frequency;
    fhro.time = hb_time;
    fhro.comment = op.comment;
  }).id;

} FC_CAPTURE_AND_RETHROW((op)) }

void_result issue_free_cycles_evaluator::do_evaluate(const issue_free_cycles_operation& op)
{ try {

  const auto& d = db();
  const auto& authority_obj = op.authority(d);
  const auto cycle_issuer_id = d.get_chain_authorities().cycle_issuer;

  // Make sure that this is the current cycle issuer:
  d.perform_chain_authority_check("cycle issuing", cycle_issuer_id, authority_obj);

  const auto& account_obj = op.account(d);
  const auto& cycle_balance_obj = d.get_cycle_balance_object(op.account);

  FC_ASSERT( account_obj.is_vault(),
             "Account '${name}' must be a vault account",
             ("name", account_obj.name)
           );

  _cycle_balance_obj = &cycle_balance_obj;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result issue_free_cycles_evaluator::do_apply(const issue_free_cycles_operation& op)
{ try {

  db().issue_cycles(*_cycle_balance_obj, op.amount);

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result issue_cycles_to_license_evaluator::do_evaluate(const operation_type& op)
{ try {

  const auto& d = db();
  const auto& authority_obj = op.authority(d);
  const auto cycle_issuer_id = d.get_chain_authorities().cycle_issuer;

  // Make sure that this is the current cycle issuer:
  d.perform_chain_authority_check("cycle issuing", cycle_issuer_id, authority_obj);

  const auto& account_obj = op.account(d);
  const auto& license = op.license(d);

  FC_ASSERT( account_obj.is_vault(),
             "Account '${name}' must be a vault account",
             ("name", account_obj.name)
  );

  FC_ASSERT ( account_obj.license_information.valid(),
              "Cannot issue cycles to a license on an account which doesn't have an issued license" );

  const auto& license_information_obj = (*account_obj.license_information)(d);

  bool found = false;
  for (uint32_t i = 0; i < license_information_obj.history.size(); ++i)
  {
    if (license_information_obj.history[i].license == op.license)
    {
      found = true;
      _frequency_lock = license_information_obj.history[i].frequency_lock;
      break;
    }
  }

  FC_ASSERT( found,
             "Cannot issue cycle to a license of type '${type}' on account ${a} because that license hasn't been issued",
             ("type", op.license)
             ("a", account_obj.name)
           );

  // Assure that amount of cycles submitted would not exceed DASCOIN_MAX_DASCOIN_SUPPLY limit.
  if (license.kind == license_kind::chartered)
  {
    FC_ASSERT(d.cycles_to_dascoin(op.amount, _frequency_lock) + d.get_total_dascoin_amount_in_system() <= DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION,
              "Cannot submit ${am} cycles with frequency (${f}), "
              "because with amount (${dsc_system} DSC in system, "
              "it would exceed DASCOIN_MAX_DASCOIN_SUPPLY limit ${dsc_max_limit} DSC",
              ("am", op.amount)
              ("f", _frequency_lock)
              ("dsc_system", d.get_total_dascoin_amount_in_system())
              ("dsc_max_limit", DASCOIN_MAX_DASCOIN_SUPPLY * DASCOIN_DEFAULT_ASSET_PRECISION)
            );
  }

  _license_information_obj = &license_information_obj;
  _kind = license.kind;
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result issue_cycles_to_license_evaluator::do_apply(const operation_type& op)
{ try {
  auto& d = db();

  if ( _kind == license_kind::regular || _kind == license_kind::locked_frequency )
  {
    d.issue_cycles(op.account, op.amount);
  }
  else if ( _kind == license_kind::chartered || _kind == license_kind::promo )
  {
    d.push_queue_submission(op.origin, {op.license}, op.account, op.amount, _frequency_lock, op.comment);
    // TODO: should we use a virtual op here?
    d.push_applied_operation(
            record_submit_charter_license_cycles_operation(op.authority, op.account, op.amount, _frequency_lock)
    );
  }

  d.modify(*_license_information_obj, [&](license_information_object& lio) {
    lio.add_non_upgradeable_cycles(op.license, op.amount);
  });

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result purchase_cycles_evaluator::do_evaluate(const operation_type& op)
{ try {

  const auto& d = db();
  const auto& wallet_obj = op.wallet_id(d);

  const dynamic_global_property_object dgpo = d.get_dynamic_global_properties();

  FC_ASSERT( dgpo.frequency == op.frequency, "Current frequency is ${cf}, ${opf} given", ("cf", dgpo.frequency)("opf", op.frequency) );

  FC_ASSERT( op.amount.asset_id == d.get_dascoin_asset_id(), "Cycles can only be purchased for DasCoin, ${a} sent", ("a", op.amount.asset_id));

  const auto& asset_obj = op.amount.asset_id(d);
  double price = static_cast<double>(op.expected_amount.value) / (static_cast<double>(dgpo.frequency.value) / DASCOIN_FREQUENCY_PRECISION);
  price = std::ceil(price * std::pow(10, asset_obj.precision)) / std::pow(10, asset_obj.precision);
  asset calculated_asset = asset(price * std::pow(10, asset_obj.precision), op.amount.asset_id);

  FC_ASSERT( calculated_asset == op.amount, "Calculated price is ${p}, but ${s} sent", ("p", d.to_pretty_string(calculated_asset))("s", d.to_pretty_string((op.amount))));

  const auto& dascoin_balance_obj = d.get_balance_object(op.wallet_id, d.get_dascoin_asset_id());

  // Check if we have enough dascoin balance:
  FC_ASSERT( dascoin_balance_obj.balance >= calculated_asset.amount,
             "Insufficient balance on wallet ${w}: ${balance}, unable to spent ${amount} on cycle purchase",
             ("w", wallet_obj.name)
             ("balance", d.to_pretty_string(dascoin_balance_obj.get_balance()))
             ("amount", d.to_pretty_string(calculated_asset))
           );

  const auto& cycle_balance_obj = d.get_balance_object(op.wallet_id, d.get_cycle_asset_id());

  _dascoin_balance_obj = &dascoin_balance_obj;
  _cycle_balance_obj = &cycle_balance_obj;

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result purchase_cycles_evaluator::do_apply(const operation_type& op)
{ try {

  auto& d = db();

  // Deduce dascoin from balance:
  d.modify(*_dascoin_balance_obj, [&](account_balance_object& acc_b){
    acc_b.balance -= op.amount.amount;
    acc_b.spent += op.amount.amount;
  });

  // Add cycles to cycle balance:
  d.modify(*_cycle_balance_obj, [&](account_balance_object& acc_b){
    acc_b.balance += op.expected_amount;
  });

  // Increase current supply:
  const auto& cycle_asset_obj = (*_cycle_balance_obj).asset_type(d);
  d.modify(cycle_asset_obj.dynamic_asset_data_id(d), [&](asset_dynamic_data_object& data){
    data.current_supply += op.expected_amount;
  });

  const auto& dgp = d.get_dynamic_global_properties();
  // If fee pool account is set, move dascoin to it:
  if (dgp.fee_pool_account_id != account_id_type())
  {
    const auto& dascoin_balance_obj = d.get_balance_object(dgp.fee_pool_account_id, d.get_dascoin_asset_id());
    d.modify(dascoin_balance_obj, [&](account_balance_object& acc_b){
      acc_b.balance += op.amount.amount;
    });
  }
  else
  {
    // Burn dascoin
    const auto& asset_obj = (*_dascoin_balance_obj).asset_type(d);
    d.modify(asset_obj.dynamic_asset_data_id(d), [&](asset_dynamic_data_object& data){
      data.current_supply -= op.amount.amount;
    });
  }

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_cycles_from_licence_to_wallet_evaluator::do_evaluate(const operation_type& op)
{ try {

  const auto& d = db();
  const auto& vault_obj = op.vault_id(d);
  const auto& wallet_obj = op.wallet_id(d);
  auto vault_kind_string = fc::reflector<account_kind>::to_string(vault_obj.kind);
  auto wallet_kind_string = fc::reflector<account_kind>::to_string(wallet_obj.kind);

  FC_ASSERT( vault_obj.is_vault(), "Cycles can be transferred only from a vault account, '${w}' is ${v}", ("w", vault_obj.name)("v", vault_kind_string) );
  FC_ASSERT( wallet_obj.is_wallet(), "Cycles can be transferred only to a wallet account, '${w}' is ${v}", ("w", wallet_obj.name)("v", wallet_kind_string) );

  FC_ASSERT( wallet_obj.is_tethered_to(op.vault_id), "Cycles can be transferred only between tethered accounts");

  FC_ASSERT ( vault_obj.license_information.valid(), "Cannot transfer cycles from a vault which doesn't have any license issued" );

  const auto& license_information_obj = (*vault_obj.license_information)(d);

  FC_ASSERT( license_information_obj.vault_license_kind != chartered, "Cannot transfer cycles from a chartered license" );

  const auto& license_iterator = std::find_if(license_information_obj.history.begin(), license_information_obj.history.end(),
                                              [&op](const license_information_object::license_history_record& history_record) {
                                                return history_record.license == op.license_id;
                                              });

  FC_ASSERT ( license_iterator != license_information_obj.history.end(), "License ${l} is not issued to account ${a}",
              ("l", op.license_id)("a", op.vault_id)
            );

  FC_ASSERT ( (*license_iterator).amount >= op.amount, "Trying to transfer ${t} cycles from license ${l} of vault ${v}, while ${r} remaining",
              ("t", op.amount)("l", op.license_id)("v", op.vault_id)("r", (*license_iterator).amount)
            );

  const auto& cycle_balance_obj = d.get_balance_object(op.wallet_id, d.get_cycle_asset_id());

  _cycle_balance_obj = &cycle_balance_obj;
  _license_information_obj = &license_information_obj;

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_cycles_from_licence_to_wallet_evaluator::do_apply(const operation_type& op)
{ try {
  auto& d = db();

  d.modify(*_license_information_obj, [&](license_information_object& lio) {
    lio.subtract_cycles(op.license_id, op.amount);
  });

  // Add cycles to wallet's cycle balance:
  d.modify(*_cycle_balance_obj, [&](account_balance_object& acc_b){
    acc_b.balance += op.amount;
  });

  // Increase current supply because at this moment cycles are becoming an asset:
  const auto cycle_id = d.get_cycle_asset_id();
  const auto& cycle_asset_obj = cycle_id(d);
  d.modify(cycle_asset_obj.dynamic_asset_data_id(d), [&](asset_dynamic_data_object& data){
    data.current_supply += op.amount;
  });

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
