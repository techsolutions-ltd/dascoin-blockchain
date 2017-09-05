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

BOOST_AUTO_TEST_CASE( set_next_spend_limit_reset_test )
{ try {

  // Determine the starting conditions - the first reset time:
  const auto START_LIMIT_RESET_TIME = db.get_dynamic_global_properties().next_spend_limit_reset;
  const auto LIMIT_INTERVAL_SECS = db.get_global_properties().parameters.limit_interval_elapse_time_seconds;

  // Get to the next time the limit is reset:
  generate_blocks(db.head_block_time() + fc::seconds(LIMIT_INTERVAL_SECS));

  // Make sure the new reset time is greater:
  const auto NEW_LIMIT_RESET_TIME = db.get_dynamic_global_properties().next_spend_limit_reset;
  BOOST_CHECK( NEW_LIMIT_RESET_TIME == START_LIMIT_RESET_TIME + fc::seconds(LIMIT_INTERVAL_SECS) );

  // Generate a single block:
  generate_block();

  // Make sure the limit reset time stays the same:
  const auto LIMIT_RESET_TIME_AFTER_BLOCK = db.get_dynamic_global_properties().next_spend_limit_reset;
  BOOST_CHECK( LIMIT_RESET_TIME_AFTER_BLOCK == NEW_LIMIT_RESET_TIME );

} FC_LOG_AND_RETHROW() }

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
  const asset ADVOCATE_EUR_LIMIT = asset(DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE, get_web_asset_id());
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

  const asset PRESIDENT_EUR_LIMIT = asset(DASCOIN_DEFAULT_EUR_LIMIT_PRESIDENT, get_web_asset_id());
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
    {_dal.get_license_type("no_license")->eur_limit, get_web_asset_id()};
  share_type expected_limit = (ADVOCATE_EUR_LIMIT * dgp.last_dascoin_price).amount;

  // Check if limit is properly set:
  const auto& balance_start = db.get_balance_object(vault_id, DASCOIN_ASSET_ID);
  BOOST_CHECK_NE( balance_start.limit.value, 0 );
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

BOOST_AUTO_TEST_CASE( daily_dascoin_price_test )
{ try {
  const auto DSC_ID = get_dascoin_asset_id();
  const auto WEBEUR_ID = get_web_asset_id();

  BOOST_CHECK_EQUAL(
    db.get_dynamic_global_properties().last_dascoin_price.to_real(),
    db.get_dynamic_global_properties().last_daily_dascoin_price.to_real()
  );
  
  set_last_dascoin_price(asset(1, DSC_ID) / asset(999999, WEBEUR_ID));

  BOOST_CHECK_NE(
    db.get_dynamic_global_properties().last_dascoin_price.to_real(),
    db.get_dynamic_global_properties().last_daily_dascoin_price.to_real()
  );

  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

  BOOST_CHECK_EQUAL(
    db.get_dynamic_global_properties().last_dascoin_price.to_real(),
    db.get_dynamic_global_properties().last_daily_dascoin_price.to_real()
  );

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

BOOST_AUTO_TEST_CASE( get_vault_info_unit_test )
{ try {
  ACTOR(wallet)
  VAULT_ACTOR(vault)

  tether_accounts(wallet_id, vault_id);

  auto res = _dal.get_vault_info(vault_id);
  BOOST_CHECK( res.valid() );
  BOOST_CHECK_EQUAL( res->eur_limit.value, DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE.value );
  BOOST_CHECK_EQUAL( res->cash_balance.value, 0 );
  BOOST_CHECK_EQUAL( res->reserved_balance.value, 0 );
  BOOST_CHECK_EQUAL( res->dascoin_balance.value, 0 );
  BOOST_CHECK_EQUAL( res->free_cycle_balance.value, 0 );
  BOOST_CHECK_EQUAL( res->spent.value, 0 );
  BOOST_CHECK( !res->license_information.valid() );

  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& dascoin_price = dgpo.last_dascoin_price;
  asset dascoin_limit = asset{DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE, db.get_web_asset_id()} * dascoin_price;

  BOOST_CHECK_EQUAL( res->dascoin_limit.value, dascoin_limit.amount.value );

  auto executive = *(_dal.get_license_type("executive"));
  time_point_sec issue_time = db.head_block_time();

  do_op(issue_license_operation(get_license_issuer_id(), vault_id, executive.id,
                                0, DASCOIN_INITIAL_FREQUENCY, issue_time));

  do_op(submit_cycles_to_queue_operation(vault_id, 1000, DASCOIN_INITIAL_FREQUENCY, "test"));

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Transfer 0.25 dascoin, we will have 0.25 in spent
  transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 0.25 * DASCOIN_DEFAULT_ASSET_PRECISION);
  dascoin_limit = asset{DASCOIN_DEFAULT_EUR_LIMIT_EXECUTIVE, db.get_web_asset_id()} * dascoin_price;

  res = _dal.get_vault_info(vault_id);
  BOOST_CHECK( res.valid() );
  BOOST_CHECK_EQUAL( res->dascoin_balance.value, db.cycles_to_dascoin(1000, DASCOIN_INITIAL_FREQUENCY).value - 0.25 * DASCOIN_DEFAULT_ASSET_PRECISION );
  // We spent 1000 cycles
  BOOST_CHECK_EQUAL( res->free_cycle_balance.value, DASCOIN_BASE_EXECUTIVE_CYCLES - 1000 );
  BOOST_CHECK_EQUAL( res->spent.value, 0.25 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( res->dascoin_limit.value, dascoin_limit.amount.value );
  BOOST_CHECK_EQUAL( res->eur_limit.value, static_cast<share_type>(DASCOIN_DEFAULT_EUR_LIMIT_EXECUTIVE).value );
  BOOST_CHECK( res->license_information.valid() );

  auto president = *(_dal.get_license_type("president"));
  issue_time = db.head_block_time();

  // Increase license to president, we should get new limit
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, president.id,
                                0, DASCOIN_INITIAL_FREQUENCY, issue_time));

  dascoin_limit = asset{DASCOIN_DEFAULT_EUR_LIMIT_PRESIDENT, db.get_web_asset_id()} * dascoin_price;
  res = _dal.get_vault_info(vault_id);
  BOOST_CHECK( res.valid() );
  // Spent remains the same:
  BOOST_CHECK_EQUAL( res->spent.value, 0.25 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( res->eur_limit.value, static_cast<share_type>(DASCOIN_DEFAULT_EUR_LIMIT_PRESIDENT).value );
  BOOST_CHECK_EQUAL( res->dascoin_limit.value, dascoin_limit.amount.value );

  // Wait for 24 hrs so spent is reset
  generate_blocks(db.head_block_time() + fc::hours(24));
  res = _dal.get_vault_info(vault_id);
  BOOST_CHECK( res.valid() );
  // Spent remains the same:
  BOOST_CHECK_EQUAL( res->spent.value, 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::limit_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
