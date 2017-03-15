/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

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

} }  // namespace graphene::chain
