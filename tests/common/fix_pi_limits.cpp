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
#include <graphene/chain/protocol/personal_identity.hpp>
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

void database_fixture::update_pi_limits(account_id_type account_id, uint8_t level, limits_type new_limits)
{
  update_pi_limits_operation op;
  op.pi_validator = get_pi_validator_id();
  op.account = account_id;
  op.level = level;
  op.new_limits = new_limits;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  db.push_transaction(trx, ~0);
}

} }  // namespace graphene::chain
