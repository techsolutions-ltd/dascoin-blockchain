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

} }  // namespace graphene::chain
