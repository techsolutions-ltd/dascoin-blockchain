/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/protocol/asset_ops.hpp>
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

const account_object& database_fixture::make_new_account_base(
   const account_kind kind,
   const account_id_type registrar,
   const string& name,
   const public_key_type& key /* = public_key_type() */)
{ try {
   account_create_operation op;

   op.kind = static_cast<uint8_t>(kind);
   op.name = name;
   op.registrar = registrar;
   op.owner = authority(123, key, 123);
   op.active = authority(321, key, 321);
   op.options.memo_key = key;
   op.options.voting_account = GRAPHENE_PROXY_TO_SELF_ACCOUNT;

   auto& active_committee_members = db.get_global_properties().active_committee_members;
   if( active_committee_members.size() > 0 )
   {
      set<vote_id_type> votes;
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      votes.insert(active_committee_members[rand() % active_committee_members.size()](db).vote_id);
      op.options.votes = flat_set<vote_id_type>(votes.begin(), votes.end());
   }
   op.options.num_committee = op.options.votes.size();

   op.fee = db.current_fee_schedule().calculate_fee( op );

   trx.operations.clear();
   trx.operations.push_back(op);
   trx.validate();
   auto r = db.push_transaction(trx, ~0);
   generate_block();
   return db.get<account_object>(r.operation_results[0].get<object_id_type>());

} FC_CAPTURE_AND_RETHROW() }

  const account_object& database_fixture::create_new_account(
     const account_id_type registrar,
     const string& name,
     const public_key_type& key /* = public_key_type() */)
  { try {

     return make_new_account_base( account_kind::wallet, registrar, name, key );

  } FC_CAPTURE_AND_RETHROW() }

  const account_object& database_fixture::create_new_vault_account(
     const account_id_type registrar,
     const string& name,
     const public_key_type& key /* = public_key_type() */)
  { try {

     return make_new_account_base( account_kind::vault, registrar, name, key );

  } FC_CAPTURE_AND_RETHROW() }

void database_fixture::tether_accounts(const account_id_type wallet, const account_id_type vault)
{ try {

  tether_accounts_operation op;
  op.wallet_account = wallet;
  op.vault_account = vault;

  set_expiration( db, trx );
  trx.operations.clear();
  trx.operations.push_back( op );
  trx.validate();
  processed_transaction ptx = db.push_transaction( trx, ~0 );

} FC_CAPTURE_AND_RETHROW ( (wallet)(vault) ) }

const account_balance_object& database_fixture::get_account_balance_object(account_id_type account_id, asset_id_type asset_id)
{
  return db.get_balance_object(account_id, asset_id);
}

} }  // namespace graphene::chain
