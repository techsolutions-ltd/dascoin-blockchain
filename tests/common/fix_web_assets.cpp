/**
 * DASCOIN!
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

const issue_asset_request_object* database_fixture::issue_webasset(account_id_type receiver_id, share_type cash,
                                                                   share_type reserved)
{ try {
  asset_create_issue_request_operation op;
  op.issuer = get_webasset_issuer_id();
  op.receiver = receiver_id;
  op.amount = cash;
  op.asset_id =  get_web_asset_id();
  op.reserved_amount = reserved;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();

  return db.find<issue_asset_request_object>(ptx.operation_results[0].get<object_id_type>());

} FC_LOG_AND_RETHROW() }

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

} }  // namespace graphene::chain
