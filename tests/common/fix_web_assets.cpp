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

namespace graphene { namespace chain {

const issue_asset_request_object* database_fixture::issue_webasset(account_id_type receiver_id, share_type cash,
                                                                   share_type reserved)
{
  asset_create_issue_request_operation op;

  op.issuer = get_webasset_issuer_id();
  op.receiver = receiver_id;
  op.amount = asset(cash, get_web_asset_id());
  op.reserved_amount = reserved;

  trx.operations.push_back( op );
  trx.validate();
  processed_transaction ptx = db.push_transaction( trx, ~0 );
  trx.operations.clear();

  return db.find<issue_asset_request_object>( ptx.operation_results[0].get<object_id_type>() );
}

} }  // namespace graphene::chain
