/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/cycle_objects.hpp>
#include <graphene/chain/protocol/cycle.hpp>
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

share_type database_fixture::get_cycle_balance(const account_id_type owner) const
{
  return db.get_cycle_balance(owner);
}

void database_fixture::adjust_cycles(const account_id_type id, const share_type amount)
{
  db.adjust_cycle_balance(id, amount);
}

const cycle_issue_request_object* database_fixture::issue_cycles(account_id_type receiver_id, share_type amount)
{ try {

  cycle_issue_request_operation op;
  op.cycle_issuer = get_cycle_issuer_id();
  op.account = receiver_id;
  op.amount = amount;

  signed_transaction tx;
  set_expiration(db, tx);
  tx.clear();
  tx.operations.push_back(op);
  tx.validate();
  processed_transaction ptx = db.push_transaction(tx, ~0);
  tx.clear();

  return db.find<cycle_issue_request_object>(ptx.operation_results[0].get<object_id_type>());

} FC_LOG_AND_RETHROW() }

void database_fixture::deny_issue_cycles(cycle_issue_request_id_type request_id)
{ try {

  cycle_issue_deny_operation op;
  op.cycle_authenticator = get_cycle_authenticator_id();
  op.request = request_id;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  db.push_transaction(trx, ~0);

} FC_LOG_AND_RETHROW() }

vector<cycle_issue_request_object> database_fixture::get_cycle_issue_request_objects_by_expiration() const
{
  vector<cycle_issue_request_object> result;
  const auto& idx = db.get_index_type<cycle_issue_request_index>().indices().get<by_expiration>();
  for ( auto req: idx )
    result.emplace_back(req);
  return result;
}

} }  // namespace graphene::chain
