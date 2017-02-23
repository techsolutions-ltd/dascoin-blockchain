/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/license_objects.hpp>
// #include <graphene/chain/account_object.hpp>
// #include <graphene/chain/asset_object.hpp>
// #include <graphene/chain/committee_member_object.hpp>
// #include <graphene/chain/fba_object.hpp>
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

const license_type_object& database_fixture::get_license_type(const string& name) const
{ try {
  const auto& idx = db.get_index_type<license_type_index>().indices().get<by_name>();
  const auto itr = idx.find(name);
  FC_ASSERT( itr != idx.end() );
  return *itr;

} FC_LOG_AND_RETHROW() }

const license_request_object* database_fixture::issue_license_to_vault_account(const account_id_type vault_account_id,
                                                                               const license_type_id_type license_id,
                                                                               share_type bonus_percentage, 
                                                                               frequency_type frequency)
{ try {
  license_request_operation op;
  op.license_issuer = get_license_issuer_id();
  op.account = vault_account_id;
  op.license = license_id;
  op.bonus_percentage = bonus_percentage,
  op.frequency = frequency;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  processed_transaction ptx = db.push_transaction(trx, ~0);
  trx.operations.clear();

  return db.find<license_request_object>( ptx.operation_results[0].get<object_id_type>() );

} FC_LOG_AND_RETHROW() }

vector<license_request_object> database_fixture::get_license_issue_requests_by_expiration() const
{ try {
  vector<license_request_object> result;

  const auto& idx = db.get_index_type<license_request_index>().indices().get<by_expiration>();
  for ( auto req: idx )
    result.emplace_back(req);

  return result;
} FC_LOG_AND_RETHROW() }

vector<license_type_object> database_fixture::get_license_history(account_id_type account_id) const
{ try {
  vector<license_type_object> result;
  const auto& acc = account_id(db);
  const auto& history = acc.license_info.history;
  result.resize(history.size());

  std::transform(history.begin(), history.end(), result.begin(),
    [this](license_information::license_history_record rec) -> license_type_object {
      return rec.license(db);
    });

  return result;

} FC_LOG_AND_RETHROW() }

const license_type_object& database_fixture::create_license_type(const string& kind, const string& name,
                                                                 share_type amount, 
                                                                 upgrade_multiplier_type balance_multipliers,
                                                                 upgrade_multiplier_type requeue_multipliers,
                                                                 upgrade_multiplier_type return_multipliers)
{ try {
  create_license_type_operation op;
  op.admin = get_license_administrator_id();
  op.name = name;
  op.amount = amount;
  op.kind = kind;
  op.balance_multipliers = balance_multipliers;
  op.requeue_multipliers = requeue_multipliers;
  op.return_multipliers = return_multipliers;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  processed_transaction ptx = db.push_transaction(trx, ~0);
  trx.operations.clear();

  return db.get<license_type_object>( ptx.operation_results[0].get<object_id_type>() );

} FC_LOG_AND_RETHROW() }

void database_fixture::edit_license_type(license_type_id_type license_id,
                                         optional<string> name, optional<share_type> amount, 
                                         optional<upgrade_multiplier_type> balance_multipliers,
                                         optional<upgrade_multiplier_type> requeue_multipliers,
                                         optional<upgrade_multiplier_type> return_multipliers)
{ try {
  license_type_edit_operation op;
  op.admin = get_license_administrator_id();
  op.license = license_id;
  op.name = name;
  op.amount = amount;
  op.balance_multipliers = balance_multipliers;
  op.requeue_multipliers = requeue_multipliers;
  op.return_multipliers = return_multipliers;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  processed_transaction ptx = db.push_transaction(trx, ~0);
  trx.operations.clear();

} FC_LOG_AND_RETHROW() }

void database_fixture::issue_license_to_vault_account(const account_object& acc, const string& lic_name, 
                                                      share_type bonus_percent, frequency_type frequency_lock)
{ try {
  auto lic = get_license_type(lic_name);
  auto req = issue_license_to_vault_account(acc.id, lic.id, bonus_percent, frequency_lock);

  BOOST_CHECK( req );
  BOOST_CHECK( req->license_issuer == get_license_issuer_id() );
  BOOST_CHECK( req->account == acc.id );
  BOOST_CHECK( req->license == lic.id );
  BOOST_CHECK( req->frequency_lock == frequency_lock );
  generate_block();

} FC_LOG_AND_RETHROW() };

void database_fixture::generate_blocks_until_license_approved()
{
  // Wait for license time to elapse:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().license_expiration_time_seconds));
};

} }  // namespace graphene::chain
