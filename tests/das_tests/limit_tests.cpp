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

BOOST_AUTO_TEST_CASE( max_dascoins_supply_limit_test )
{
  try
  {
    // First account should hold 8589709593 reserved cycles
    // Second account should hold Standard License Cycle amount 1100 plus 10% (1210)
    VAULT_ACTORS((first)(second))
    ACTOR(wallet)

    // Frequency is 1 (easier math)
    adjust_frequency(1 * DASCOIN_FREQUENCY_PRECISION);

    // System has 0 DSC issued
    BOOST_CHECK_EQUAL(db.get_total_dascoin_amount_in_system().value, 0);

    // Required to throw cause amount of cycles issued would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, DASCOIN_MAX_DASCOIN_SUPPLY + 1, 1 * DASCOIN_FREQUENCY_PRECISION, "")), fc::exception);

    // Issue 8589709593 reserved cycles to first account
    do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 8589709593, 1 * DASCOIN_FREQUENCY_PRECISION, ""));

    // Check to see if total amount of coins is 8589709593
    BOOST_CHECK_EQUAL(db.get_total_dascoin_amount_in_system().value / DASCOIN_DEFAULT_ASSET_PRECISION, 8589709593);

    // Required to fail cause issuing 225000 would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 225000, 1 * DASCOIN_FREQUENCY_PRECISION, "")), fc::exception);

    auto standard_charter = *(_dal.get_license_type("standard_charter"));
    auto vp_locked = *(_dal.get_license_type("vice_president_locked"));

    variant v;
    fc::to_variant(vp_locked.id, v);

    // Issue VP_LOCKED license for first account
    do_op(issue_license_operation(get_license_issuer_id(), first_id, vp_locked.id, 10, 1 * DASCOIN_FREQUENCY_PRECISION, db.head_block_time()));

    // Required to fail cause submitting 225000 to queue would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_cycles_to_queue_operation(first_id, 225000, 1 * DASCOIN_FREQUENCY_PRECISION, v.as_string())), fc::exception);

    // Required to fail cause submitting 225000 to queue would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_cycles_to_queue_by_license_operation(first_id, 225000, vp_locked.id, 1 * DASCOIN_FREQUENCY_PRECISION, "TEST")), fc::exception);

    // Issue PRESIDENT_CHARTER license for second account
    auto p_charter = *(_dal.get_license_type("president_charter"));

    // Required to fail cause issuing this license would submit cycles to queue which would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(issue_license_operation(get_license_issuer_id(), second_id, p_charter.id, 10, 1 * DASCOIN_FREQUENCY_PRECISION, db.head_block_time())), fc::exception);

    // Issue STANDARD_CHARTER license for second account
    do_op(issue_license_operation(get_license_issuer_id(), second_id, standard_charter.id, 10, 1 * DASCOIN_FREQUENCY_PRECISION, db.head_block_time()));

    // Required to fail cause issuing additional cycles to second account license would exceed DASCOIN_MAX_DASCOIN_SUPPLY
//    GRAPHENE_REQUIRE_THROW(do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), second_id, standard_charter.id, 225000, "ORIGIN", "COMMENT")), fc::exception );


    // Empty reward queue
    adjust_dascoin_reward(1000000000 * DASCOIN_DEFAULT_ASSET_PRECISION);
    BOOST_CHECK_EQUAL(db.get_total_dascoin_amount_in_system().value / DASCOIN_DEFAULT_ASSET_PRECISION, 8589709593 + 1210);
    BOOST_CHECK_EQUAL(_dal.get_reward_queue_size(), 2);

    auto queue = _dal.get_reward_queue();
    BOOST_CHECK_EQUAL(queue[0].amount.value, 8589709593);
    BOOST_CHECK_EQUAL(queue[1].amount.value, 1210);

    toggle_reward_queue(true);
    for (int i = 0; i < 9; i++)
      generate_blocks(db.head_block_time() + fc::seconds(600));

    BOOST_CHECK_EQUAL(_dal.get_reward_queue().size(), 0);
    BOOST_CHECK_EQUAL(db.get_total_dascoin_amount_in_system().value / DASCOIN_DEFAULT_ASSET_PRECISION, 8589709593 + 1210);

    BOOST_CHECK_EQUAL(get_balance(first_id, get_dascoin_asset_id()), 8589709593 * DASCOIN_DEFAULT_ASSET_PRECISION);
    BOOST_CHECK_EQUAL(get_balance(second_id, get_dascoin_asset_id()), 1210 * DASCOIN_DEFAULT_ASSET_PRECISION);

    // Queue is empty, amount of DSC in system is in total_dascoin_minted, repeat tests..

    // Required to fail cause submitting 225000 would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 225000, 1 * DASCOIN_FREQUENCY_PRECISION, "")), fc::exception);

    // Required to fail cause submitting 225000 to queue would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_cycles_to_queue_operation(first_id, 225000, 1 * DASCOIN_FREQUENCY_PRECISION, v.as_string())), fc::exception);

    // Required to fail cause submitting 225000 to queue would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(submit_cycles_to_queue_by_license_operation(first_id, 225000, vp_locked.id, 1 * DASCOIN_FREQUENCY_PRECISION, "TEST")), fc::exception);

    // Required to fail cause issuing this license would submit cycles to queue which would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(issue_license_operation(get_license_issuer_id(), second_id, p_charter.id, 10, 1 * DASCOIN_FREQUENCY_PRECISION, db.head_block_time())), fc::exception);

    // Required to fail cause issuing additional cycles to second account license would exceed DASCOIN_MAX_DASCOIN_SUPPLY
    GRAPHENE_REQUIRE_THROW(do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), second_id, standard_charter.id, 225000, "ORIGIN", "COMMENT")), fc::exception );

  }
  FC_LOG_AND_RETHROW()
}

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

  BOOST_CHECK( !vault.disable_vault_to_wallet_limit );

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

BOOST_AUTO_TEST_CASE( toggle_limit_dascoin_test )
{ try {
  VAULT_ACTOR(sender);
  ACTOR(receiver);

  tether_accounts(receiver_id, sender_id);

  BOOST_CHECK( !sender.disable_vault_to_wallet_limit );

  // Give this account a bunch of dascoin and disable sending limit:
  issue_dascoin(sender_id, 10000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL( get_balance(sender_id, get_dascoin_asset_id()), 10000 * DASCOIN_DEFAULT_ASSET_PRECISION );
  disable_vault_to_wallet_limit(sender_id);
  BOOST_CHECK( sender.disable_vault_to_wallet_limit );

  transfer_dascoin_vault_to_wallet(sender_id, receiver_id, 10000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL( get_dascoin_balance(receiver_id), 10000 * DASCOIN_DEFAULT_ASSET_PRECISION );

  issue_dascoin(sender_id, 10000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL( get_balance(sender_id, get_dascoin_asset_id()), 10000 * DASCOIN_DEFAULT_ASSET_PRECISION );
  enable_vault_to_wallet_limit(sender_id);
  BOOST_CHECK( !sender.disable_vault_to_wallet_limit );

  // This will fail:
  GRAPHENE_REQUIRE_THROW( transfer_dascoin_vault_to_wallet(sender_id, receiver_id, 10000 * DASCOIN_DEFAULT_ASSET_PRECISION), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( update_euro_limit_operation_test )
{ try {
  VAULT_ACTOR(sender);
  ACTOR(receiver);

  tether_accounts(receiver_id, sender_id);
  issue_dascoin(sender_id, 10000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  BOOST_CHECK_EQUAL( get_dascoin_balance(sender_id), 10000 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK( !sender.disable_vault_to_wallet_limit );

  // Turn off limit:
  push_op(update_euro_limit_operation(get_license_administrator_id(), sender_id, true, {}, "foo"));
  BOOST_CHECK_EQUAL( get_dascoin_balance(sender_id), 10000 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_dascoin_balance(receiver_id), 0 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK( sender.disable_vault_to_wallet_limit );

  // Transfer 5,000 das:
  transfer_dascoin_vault_to_wallet(sender_id, receiver_id, 5000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL( get_dascoin_balance(sender_id), 5000 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_dascoin_balance(receiver_id), 5000 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Turn limit back on:
  push_op(update_euro_limit_operation(get_license_administrator_id(), sender_id, false, {}, "foo"));

  BOOST_CHECK( !sender.disable_vault_to_wallet_limit );

  // This will fail:
  GRAPHENE_REQUIRE_THROW( transfer_dascoin_vault_to_wallet(sender_id, receiver_id, 5000 * DASCOIN_DEFAULT_ASSET_PRECISION), fc::exception );

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

  auto executive = *(_dal.get_license_type("executive_locked"));
  variant v;
  fc::to_variant(executive.id, v);

  time_point_sec issue_time = db.head_block_time();

  do_op(issue_license_operation(get_license_issuer_id(), vault_id, executive.id,
                                0, DASCOIN_INITIAL_FREQUENCY, issue_time));

  do_op(submit_cycles_to_queue_operation(vault_id, 1000, DASCOIN_INITIAL_FREQUENCY, v.as_string()));

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

  auto president = *(_dal.get_license_type("president_locked"));
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
