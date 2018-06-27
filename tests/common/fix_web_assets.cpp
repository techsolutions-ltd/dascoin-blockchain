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

#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/protocol/asset_ops.hpp>
#include <graphene/chain/protocol/wire.hpp>
#include <graphene/chain/wire_object.hpp>
#include <graphene/chain/protocol/wire_out_with_fee.hpp>
#include <graphene/chain/wire_out_with_fee_object.hpp>
#include <graphene/chain/issued_asset_record_object.hpp>
// #include <graphene/chain/account_object.hpp>
// #include <graphene/chain/committee_member_object.hpp>
// #include <graphene/chain/fba_object.hpp>
// #include <graphene/chain/license_objects.hpp>
// #include <graphene/chain/market_object.hpp>
// #include <graphene/chain/vesting_balance_object.hpp>
// #include <graphene/chain/witness_object.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

asset database_fixture::web_asset(share_type amount)
{
  return asset(amount, get_web_asset_id());
}

const issued_asset_record_object* database_fixture::issue_asset(const string& unique_id, account_id_type receiver_id,
                                                                share_type cash, share_type reserved,
                                                                asset_id_type asset, account_id_type issuer, string comment)
{ try {
  asset_create_issue_request_operation op;
  op.unique_id = unique_id;
  op.issuer = issuer;
  op.receiver = receiver_id;
  op.amount = cash;
  op.asset_id =  asset;
  op.reserved_amount = reserved;
  op.comment = comment;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();

  return db.find<issued_asset_record_object>(ptx.operation_results[0].get<object_id_type>());

} FC_LOG_AND_RETHROW() }

const issued_asset_record_object* database_fixture::issue_webasset(const string& unique_id, account_id_type receiver_id,
                                                                   share_type cash, share_type reserved)
{
  return issue_asset(unique_id, receiver_id, cash, reserved, get_web_asset_id(), get_webasset_issuer_id(), "TEST_ISSUE_WEB_ASSET");
}

const issued_asset_record_object* database_fixture::issue_cycleasset(const string& unique_id, account_id_type receiver_id,
                                                                     share_type cash, share_type reserved)
{
  return issue_asset(unique_id, receiver_id, cash, reserved, get_cycle_asset_id(), get_webasset_issuer_id(), "TEST_ISSUE_CYCLE_ASSET");
}

std::pair<share_type, share_type> database_fixture::get_web_asset_amounts(account_id_type owner_id)
{
  const auto& balance_obj = db.get_balance_object(owner_id, get_web_asset_id());
  return std::make_pair(balance_obj.balance, balance_obj.reserved);
}

std::pair<asset, asset> database_fixture::get_web_asset_balances(account_id_type owner_id)
{
  share_type cash, reserved;
  std::tie(cash, reserved) = get_web_asset_amounts(owner_id);
  return std::make_pair(asset(cash, get_web_asset_id()), asset(reserved, get_web_asset_id()));
}

void database_fixture::transfer_webasset_vault_to_wallet(account_id_type vault_id, account_id_type wallet_id,
                                                         std::pair<share_type, share_type> amounts)
{ try {
  share_type cash, reserved;
  std::tie(cash, reserved) = amounts;

  transfer_vault_to_wallet_operation op;
  op.from_vault = vault_id;
  op.to_wallet = wallet_id;
  op.asset_to_transfer = asset(cash, db.get_web_asset_id());
  op.reserved_to_transfer = reserved;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  db.push_transaction(trx, ~0);

} FC_LOG_AND_RETHROW() }

void database_fixture::transfer_webasset_wallet_to_vault(account_id_type wallet_id, account_id_type vault_id,
                                                         std::pair<share_type, share_type> amounts)
{ try {
  share_type cash, reserved;
  std::tie(cash, reserved) = amounts;

  transfer_wallet_to_vault_operation op;
  op.from_wallet = wallet_id;
  op.to_vault = vault_id;
  op.asset_to_transfer = asset(cash, db.get_web_asset_id());
  op.reserved_to_transfer = reserved;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  db.push_transaction(trx, ~0);

} FC_LOG_AND_RETHROW() }

void database_fixture::transfer_dascoin_vault_to_wallet(account_id_type vault_id, account_id_type wallet_id,
                                                        share_type amount)
{ try {
    transfer_vault_to_wallet_operation op;
    op.from_vault = vault_id;
    op.to_wallet = wallet_id;
    op.asset_to_transfer = asset(amount, db.get_dascoin_asset_id());
    op.reserved_to_transfer = 0;

    set_expiration(db, trx);
    trx.operations.clear();
    trx.operations.push_back(op);
    trx.validate();
    db.push_transaction(trx, ~0);

} FC_LOG_AND_RETHROW() }

void database_fixture::transfer_dascoin_wallet_to_vault(account_id_type wallet_id, account_id_type vault_id,
                                                        share_type amount)
{ try {
    transfer_wallet_to_vault_operation op;
    op.from_wallet = wallet_id;
    op.to_vault = vault_id;
    op.asset_to_transfer = asset(amount, db.get_dascoin_asset_id());
    op.reserved_to_transfer = 0;

    set_expiration(db, trx);
    trx.operations.clear();
    trx.operations.push_back(op);
    trx.validate();
    db.push_transaction(trx, ~0);

} FC_LOG_AND_RETHROW() }

void database_fixture::deny_issue_request(issue_asset_request_id_type request_id)
{ try {

  asset_deny_issue_request_operation op;
  op.authenticator = get_webasset_authenticator_id();
  op.request = request_id;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  db.push_transaction(trx, ~0);

} FC_LOG_AND_RETHROW() }

vector<issue_asset_request_object> database_fixture::get_asset_request_objects(account_id_type account_id)
{ try {
  vector<issue_asset_request_object> result;

  const auto& idx = db.get_index_type<issue_asset_request_index>().indices().get<by_account_asset>();
  for( auto itr = idx.find(boost::make_tuple(account_id)); itr != idx.end() && itr->receiver == account_id; itr++)
    result.emplace_back(*itr);

  return result;

} FC_LOG_AND_RETHROW() }

share_type database_fixture::get_asset_current_supply(asset_id_type asset_id)
{ try {

  auto asset = db.get<asset_object>(asset_id);
  return asset.dynamic_asset_data_id(db).current_supply;

} FC_LOG_AND_RETHROW() }

void database_fixture::set_last_dascoin_price(price val)
{ try {

  db.modify(get_dynamic_global_properties(), [val](dynamic_global_property_object& dgpo){
    dgpo.last_dascoin_price = val;
  });

} FC_LOG_AND_RETHROW() }

void database_fixture::set_last_daily_dascoin_price(price val)
{ try {

  db.modify(get_dynamic_global_properties(), [val](dynamic_global_property_object& dgpo){
    dgpo.last_daily_dascoin_price = val;
  });

} FC_LOG_AND_RETHROW() }

void database_fixture::issue_dascoin(account_object& vault_obj, share_type amount)
{ try {
  issue_dascoin(vault_obj.id, amount);
} FC_LOG_AND_RETHROW() }

void database_fixture::issue_dascoin(account_id_type vault_id, share_type amount)
{ try {  
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, amount, 100, ""));
  toggle_reward_queue(true);

  auto num_intervals = (amount * DASCOIN_DEFAULT_ASSET_PRECISION) / get_global_properties().parameters.dascoin_reward_amount + 1;
  generate_blocks(db.head_block_time() + fc::seconds(get_global_properties().parameters.reward_interval_time_seconds * num_intervals.value), false);

} FC_LOG_AND_RETHROW() }

void database_fixture::mint_all_dascoin_from_license(license_type_id_type license, account_id_type vault_id, account_id_type wallet_id,
                                                          share_type bonus, share_type frequency_lock)
{ try {
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, license, bonus, frequency_lock, db.head_block_time()));
  auto license_information = db.get_license_information(vault_id);

  if (license_information.valid())
  {
    toggle_reward_queue(true);
    auto last_issued_license = license_information->history.back();
    if (license_information->is_manual_submit())
    {
      do_op(submit_cycles_to_queue_by_license_operation(vault_id, last_issued_license.amount, license, frequency_lock, "TEST"));
    }
    auto num_intervals = (last_issued_license.amount * DASCOIN_DEFAULT_ASSET_PRECISION) / get_global_properties().parameters.dascoin_reward_amount + 1;
    generate_blocks(db.head_block_time() + fc::seconds(get_global_properties().parameters.reward_interval_time_seconds * num_intervals.value), false);

    if (wallet_id != account_id_type())
    {
      tether_accounts(wallet_id, vault_id);
      disable_vault_to_wallet_limit(vault_id);
      transfer_dascoin_vault_to_wallet(vault_id, wallet_id, db.cycles_to_dascoin(last_issued_license.amount, frequency_lock));
      enable_vault_to_wallet_limit(vault_id);
    }
    toggle_reward_queue(false);
  }

} FC_LOG_AND_RETHROW() }

asset_id_type database_fixture::create_new_asset(const string& symbol, share_type max_supply, uint8_t precision, const price& core_exchange_rate)
{ try {

    auto issuer_id = db.get_global_properties().authorities.webasset_issuer;

    asset_id_type test_asset_id = db.get_index<asset_object>().get_next_id();
    asset_create_operation creator;
    creator.issuer = issuer_id;
    creator.symbol = symbol;
    creator.common_options.max_supply = max_supply;
    creator.precision = precision;
    creator.common_options.core_exchange_rate = core_exchange_rate;
    do_op(creator);

    return test_asset_id;
} FC_LOG_AND_RETHROW() }

} }  // namespace graphene::chain
