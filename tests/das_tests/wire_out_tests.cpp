/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/wire_object.hpp>
#include <graphene/chain/protocol/wire.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( wire_out_web_asset_test )
{ try {
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(vault);
  generate_block();

  const auto check_balances = [this](const account_object& account, share_type expected_cash,
                                     share_type expected_reserved)
  {
    share_type cash, reserved;
    std::tie(cash, reserved) = get_web_asset_amounts(account.id);

    BOOST_CHECK_EQUAL( cash.value, expected_cash.value );
    BOOST_CHECK_EQUAL( reserved.value, expected_reserved.value );
  };

  // Reject, insuficcient balance:
  GRAPHENE_REQUIRE_THROW( wire_out(wallet_id, web_asset(10000)), fc::exception );

  issue_webasset("1", wallet_id, 15000, 15000);
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1)); // TODO: use global propety.

  // Update the limits:
  // update_pi_limits(wallet_id, 99, {20000,20000,20000});

  // Wire out 10K:
  wire_out(wallet_id, web_asset(10000), "debit");

  // Check if the balance has been reduced:
  check_balances(wallet, 5000, 15000);

  // Check if the holder object exists:
  auto holders = get_wire_out_holders(wallet_id, {get_web_asset_id()});
  BOOST_CHECK_EQUAL( holders.size(), 1 );
  BOOST_CHECK_EQUAL( holders[0].memo, "debit" );

  // Wire out 5K:
  wire_out(wallet_id, web_asset(5000));

  // Check the balances are zero:
  check_balances(wallet, 0, 15000);

  // There should be two holders now:
  holders = get_wire_out_holders(wallet_id, {get_web_asset_id()});
  BOOST_CHECK_EQUAL(holders.size(), 2);

  // Deny the first request:
  wire_out_reject(holders[0].id);

  // Check if the wire out holder was deleted:
  BOOST_CHECK_EQUAL( get_wire_out_holders(wallet_id, {get_web_asset_id()}).size(), 1 );

  // 10K should return to the wallet:
  check_balances(wallet, 10000, 15000);

  // Complete a wire out transaction:
  wire_out_complete(holders[1].id);

  // Check if the wire out holder object was deleted:
  BOOST_CHECK_EQUAL( get_wire_out_holders(wallet_id, {get_web_asset_id()}).size(), 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
