/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/access_layer.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/license_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )
BOOST_FIXTURE_TEST_SUITE( limit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( get_dascoin_limit_unit_test )
{ try {
  const share_type WEB_AMOUNT = 10 * DASCOIN_FIAT_ASSET_PRECISION;
  const share_type DAS_AMOUNT = 100 * DASCOIN_DEFAULT_ASSET_PRECISION;
  // We use the DAS:WEB market.
  const price DASCOIN_PRICE = 
     asset{DAS_AMOUNT, get_dascoin_asset_id()} / asset{WEB_AMOUNT, get_web_asset_id()};

  optional<share_type> returned_limit;
  optional<share_type> expected_limit;

  // For a wallet account, return nothing:
  ACTOR(wallet);
  returned_limit = db.get_dascoin_limit(wallet, DASCOIN_PRICE);
  BOOST_CHECK(!returned_limit.valid());

  // For a vault advocate. return limit based on advocate eur_limit:
  VAULT_ACTOR(advocate);
  const asset ADVOCATE_EUR_LIMIT = asset(DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE, get_dascoin_asset_id());
  expected_limit = (ADVOCATE_EUR_LIMIT * DASCOIN_PRICE).amount;
  returned_limit = db.get_dascoin_limit(advocate, DASCOIN_PRICE);
  BOOST_CHECK(returned_limit.valid());
  BOOST_CHECK_EQUAL(returned_limit->value, expected_limit->value);

  //For a licensed vault account, return limit equal to the appropriate license eur_limit:
  VAULT_ACTOR(president);
  const auto pres_lic = *(_dal.get_license_type("president"));
  const share_type bonus_percent = 50;
  const share_type frequency_lock = 0;
  const time_point_sec issue_time = db.head_block_time();
  do_op(issue_license_operation(get_license_issuer_id(), president_id, pres_lic.id,
        bonus_percent, frequency_lock, issue_time));
  BOOST_CHECK( president.license_information.valid() );

  const asset PRESIDENT_EUR_LIMIT = asset(DASCOIN_DEFAULT_EUR_LIMIT_PRESIDENT, get_dascoin_asset_id());
  expected_limit = (PRESIDENT_EUR_LIMIT * DASCOIN_PRICE).amount;
  returned_limit = db.get_dascoin_limit(president, DASCOIN_PRICE);
  BOOST_CHECK(returned_limit.valid());
  BOOST_CHECK_EQUAL(returned_limit->value, expected_limit->value);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( adjust_balance_limit_unit_test )
{ try {
  const auto DASCOIN_ASSET_ID = get_dascoin_asset_id();
  const auto LIMIT = 100;
  VAULT_ACTOR(vault);

  db.adjust_balance_limit(vault, DASCOIN_ASSET_ID, LIMIT);

  const auto& balance = db.get_balance_object(vault_id, DASCOIN_ASSET_ID);
  BOOST_CHECK_EQUAL( balance.limit.value, LIMIT );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( limit_reset_test )
{ try {
  const auto DASCOIN_ASSET_ID = get_dascoin_asset_id();
  VAULT_ACTOR(vault);

  auto& dgp = db.get_dynamic_global_properties();
  const asset ADVOCATE_EUR_LIMIT = 
    {_dal.get_license_type("no_license")->eur_limit, DASCOIN_ASSET_ID};
  share_type expected_limit = (ADVOCATE_EUR_LIMIT * dgp.last_dascoin_price).amount;

  // Check if limit is properly set:
  const auto& balance_start = db.get_balance_object(vault_id, DASCOIN_ASSET_ID);
  BOOST_CHECK_EQUAL( balance_start.limit.value, expected_limit.value );

  // Change the limit:
  const auto& balance_zero_limit = db.get_balance_object(vault_id, DASCOIN_ASSET_ID);
  db.adjust_balance_limit(vault, DASCOIN_ASSET_ID, 1);
  BOOST_CHECK_EQUAL( balance_zero_limit.limit.value, 1 );

  // Wait for the limit interval to pass:
  generate_blocks(dgp.next_spend_limit_reset + fc::seconds(10));

  // Check the limit again:
  const auto& balance_reset = db.get_balance_object(vault_id, DASCOIN_ASSET_ID);
  BOOST_CHECK_EQUAL( balance_reset.limit.value, expected_limit.value );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( obey_limit_test )
{ try {
  ACTOR(wallet);
  VAULT_ACTOR(vault);

  tether_accounts(wallet_id, vault_id);

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, 200, 200, ""));
  toggle_reward_queue(true);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

  // Set limit to 10 dascoin
  db.adjust_balance_limit(vault, get_dascoin_asset_id(), 10 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // This ought to pass
  transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 10 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // This will fail
  GRAPHENE_REQUIRE_THROW( transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 20 * DASCOIN_DEFAULT_ASSET_PRECISION), fc::exception );

  // Wait for the limit interval to pass:
  auto& dgp = db.get_dynamic_global_properties();
  generate_blocks(dgp.next_spend_limit_reset + fc::seconds(10));

  // Set limit to 20 dascoin
  db.adjust_balance_limit(vault, get_dascoin_asset_id(), 20 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Now this ought to pass
  transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 20 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // This will fail
  GRAPHENE_REQUIRE_THROW( transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 1 * DASCOIN_DEFAULT_ASSET_PRECISION), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::limit_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
