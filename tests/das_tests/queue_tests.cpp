/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/access_layer.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>

#include <graphene/chain/queue_objects.hpp>

#include "../common/database_fixture.hpp"

#include <cinttypes>

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( queue_tests, database_fixture )

BOOST_AUTO_TEST_CASE( convert_dascoin_cycles_precision_test )
{
  auto amount = db.cycles_to_dascoin(10000, 200);
  BOOST_CHECK_EQUAL(amount.value, 5000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  amount = db.dascoin_to_cycles(5000 * DASCOIN_DEFAULT_ASSET_PRECISION, 200);
  BOOST_CHECK_EQUAL(amount.value, 10000);
}

BOOST_AUTO_TEST_CASE( convert_dascoin_cycles_test )
{
  // TODO: do not use hardcoded values.
  share_type amount = db.cycles_to_dascoin(10000, 300);
  BOOST_CHECK_EQUAL(amount.value, 333333333);

  amount = db.dascoin_to_cycles(333333333, 300);
  BOOST_CHECK_EQUAL(amount.value, 9999);
}

BOOST_AUTO_TEST_CASE( dascoin_reward_amount_regression_test )
{ try {

  share_type amount = 600000 * DASCOIN_DEFAULT_ASSET_PRECISION;
  BOOST_CHECK( amount > INT32_MAX );
  BOOST_CHECK( amount < INT64_MAX );
  BOOST_CHECK( amount == 600000 * DASCOIN_DEFAULT_ASSET_PRECISION );
  do_op(update_queue_parameters_operation(get_license_issuer_id(), {true}, {600}, {amount}));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( update_queue_parameters_unit_test )
{ try {

  // Before HARDFORK_BLC_58_TIME, we submit reward amount divided by 10:
  do_op(update_queue_parameters_operation(get_license_issuer_id(), {true}, {600}, 
                                          {200 * DASCOIN_DEFAULT_ASSET_PRECISION}));

  const auto& params = get_chain_parameters();
  BOOST_CHECK_EQUAL( params.enable_dascoin_queue, true );
  BOOST_CHECK_EQUAL( params.reward_interval_time_seconds, 600 );
  BOOST_CHECK_EQUAL( params.dascoin_reward_amount.value, 2000 * DASCOIN_DEFAULT_ASSET_PRECISION );

  generate_blocks( HARDFORK_BLC_58_TIME );

  // After HARDFORK_BLC_58_TIME, reward amount is submitted with precision of 5:
  do_op(update_queue_parameters_operation(get_license_issuer_id(), {true}, {600},
                                          {2000 * DASCOIN_DEFAULT_ASSET_PRECISION}));

  const auto& params2 = get_chain_parameters();
  BOOST_CHECK_EQUAL( params2.dascoin_reward_amount.value, 2000 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // TODO: handle negative cases

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_chartered_license_unit_test )
{ try {
  VAULT_ACTOR(first)

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), first_id, lic_typ.id,
      10, 200, db.head_block_time()));

  auto result_vec = *_dal.get_queue_submissions_with_pos(first_id).result;
  BOOST_CHECK_EQUAL( result_vec.size(), 1 );

  BOOST_CHECK_EQUAL( result_vec[0].position, 0 );

  auto rqo = result_vec[0].submission;
  BOOST_CHECK_EQUAL( rqo.origin, "charter_license" );
  BOOST_CHECK( rqo.license.valid() );
  BOOST_CHECK( *rqo.license == lic_typ.id );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 1210 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( submit_cycles_operation_test )
{ try {
  VAULT_ACTOR(first)

  adjust_frequency(200);

  adjust_cycles(first_id, 200);
  variant v;
  auto locked = *(_dal.get_license_type("standard_locked"));
  fc::to_variant(locked.id, v);
  const share_type bonus_percent = 50;
  share_type frequency_lock = 200;
  const time_point_sec issue_time = db.head_block_time();

  // This will fail, frequency cannot be zero:
  do_op(issue_license_operation(get_license_issuer_id(), first_id, locked.id,
                                bonus_percent, frequency_lock, issue_time));

  // Error: wrong frequency.
  GRAPHENE_REQUIRE_THROW(
    do_op(submit_cycles_to_queue_operation(first_id, 100, 730, v.as_string())),
    fc::exception
  );

  do_op(submit_cycles_to_queue_operation(first_id, 100, 200, v.as_string()));

  auto result_vec = *_dal.get_queue_submissions_with_pos(first_id).result;
  BOOST_CHECK_EQUAL( result_vec.size(), 1 );

  BOOST_CHECK_EQUAL( result_vec[0].position, 0 );

  auto rqo = result_vec[0].submission;
  BOOST_CHECK_EQUAL( rqo.origin, "user_submit" );
  BOOST_CHECK( rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 100 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( basic_submit_reserved_cycles_to_queue_test )
{ try {
  VAULT_ACTOR(first)
  VAULT_ACTOR(second)
  VAULT_ACTOR(third)
  VAULT_ACTOR(fourth)

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 400, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 600, 200, ""));

  // Queue looks like this:
  // 200 --> 400 --> 200 --> 600

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 4 );

  toggle_reward_queue(true);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 100, 200, 100, 100

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 1 );

  // Wait for the rest if the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }
/*
BOOST_AUTO_TEST_CASE( basic_submit_cycles_to_queue_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  adjust_cycles(first_id, 200);
  adjust_cycles(second_id, 400);
  adjust_cycles(third_id, 200);
  adjust_cycles(fourth_id, 600);

  do_op(submit_cycles_to_queue_operation(first_id, 200, 200, "TEST"));
  do_op(submit_cycles_to_queue_operation(second_id, 400, 200, "TEST"));
  do_op(submit_cycles_to_queue_operation(third_id, 200, 200, "TEST"));
  do_op(submit_cycles_to_queue_operation(fourth_id, 600, 200, "TEST"));

  // Queue looks like this:
  // 200 --> 400 --> 200 --> 600

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 4 );

  toggle_reward_queue(true);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 100, 200, 100, 100

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 1 );

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }
*/
BOOST_AUTO_TEST_CASE( basic_chartered_license_to_queue_test )
{ 
  const auto& issue =  [&](account_id_type account_id, const string& license_name, share_type bonus_percentage, 
                           share_type frequency_lock)
  {
    auto lic_typ = *(_dal.get_license_type(license_name));
    do_op(issue_license_operation(get_license_issuer_id(), account_id, lic_typ.id,
        bonus_percentage, frequency_lock, db.head_block_time()));
  };

  try {

  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(5000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  issue(first_id, "standard_charter", 100, 200);  // 1100 + 1 * 1100 = 2200 cycles
  issue(second_id, "standard_charter", 300, 200);  // 1100 + 3 * 1100 = 4400 cycles
  issue(third_id, "standard_charter", 100, 200);  // 1100 + 1 * 1100 = 2200 cycles
  issue(fourth_id, "standard_charter", 500, 200);  // 1100 + 5 * 1100 = 6600 cycles

  // Queue looks like this:
  // 2200 --> 4400 --> 2200 --> 6600

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 4 );

  toggle_reward_queue(true);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 1100, 2200, 1100, 600

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 1100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 2200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 1100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 600 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 1 );

  // Wait for the rest if the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 1100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 2200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 1100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 3300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( submit_cycles_to_queue_by_license_operation_test )
{ try {
  VAULT_ACTOR(vault)

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto manager_locked = *(_dal.get_license_type("manager_locked"));
  const share_type bonus_percent = 10;
  share_type frequency_lock = 200;
  const time_point_sec issue_time = db.head_block_time();

  // Error: no issued license.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_by_license_operation(vault_id, 100, standard_locked.id, 200, "TEST")), fc::exception );

  do_op(issue_license_operation(get_license_issuer_id(), vault_id, manager_locked.id, bonus_percent, frequency_lock, issue_time));

  // Error: issued other license.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_by_license_operation(vault_id, 100, standard_locked.id, 200, "TEST")), fc::exception );

  // Error: cannot submit zero cycles.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_by_license_operation(vault_id, 0, manager_locked.id, 200, "TEST")), fc::exception );

  // Error: not enough cycles on the balance.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_by_license_operation(vault_id, 2 * DASCOIN_BASE_MANAGER_CYCLES, manager_locked.id, 200, "TEST")), fc::exception );

  // Error: frequency (20) is not equal to license's frequency.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_by_license_operation(vault_id, 1000, manager_locked.id, 20, "TEST")), fc::exception );

  do_op(submit_cycles_to_queue_by_license_operation(vault_id, 1000, manager_locked.id, 200, "TEST"));

  const auto& license_information_obj = (*vault.license_information)(db);
  const auto& license_history = license_information_obj.history;
  const auto& license_record = license_history[0];
  const uint32_t remaining_cycles = static_cast<uint32_t>(DASCOIN_BASE_MANAGER_CYCLES * 1.1) - 1000;
  BOOST_CHECK_EQUAL( license_record.amount.value, remaining_cycles );

  const auto& balance = get_cycle_balance(vault_id);
  BOOST_CHECK_EQUAL( balance.value, remaining_cycles );

  // Error: not enough cycles on the balance:
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_by_license_operation(vault_id, remaining_cycles + 1, manager_locked.id, 200, "TEST")), fc::exception );

  do_op(submit_cycles_to_queue_by_license_operation(vault_id, 1000, manager_locked.id, 200, "TEST"));

  const auto& license_information_obj2 = (*vault.license_information)(db);
  const auto& license_history2 = license_information_obj2.history;
  const auto& license_record2 = license_history2[0];
  BOOST_CHECK_EQUAL( license_record2.amount.value, remaining_cycles - 1000 );

  const auto& balance2 = get_cycle_balance(vault_id);
  BOOST_CHECK_EQUAL( balance2.value, remaining_cycles - 1000 );

  // Issue 200 cycles:
  do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), vault_id, manager_locked.id, 200, "foo", "bar"));

  // Submit 100:
  do_op(submit_cycles_to_queue_by_license_operation(vault_id, 100, manager_locked.id, 200, "TEST"));

  const auto& license_information_obj3 = (*vault.license_information)(db);
  const auto& license_history3 = license_information_obj3.history;
  const auto& license_record3 = license_history3[0];

  // Cycles are first submitted from non_upgradeable_amount:
  BOOST_CHECK_EQUAL( license_record3.non_upgradeable_amount.value, 100 );
  const auto& balance3 = get_cycle_balance(vault_id);
  BOOST_CHECK_EQUAL( balance3.value, remaining_cycles - 900 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( submit_cycles_to_queue_operation_test )
{ try {
  VAULT_ACTOR(vault)

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto manager_locked = *(_dal.get_license_type("manager_locked"));
  variant vs;
  variant vm;
  fc::to_variant(standard_locked.id, vs);
  fc::to_variant(manager_locked.id, vm);

  const share_type bonus_percent = 10;
  share_type frequency_lock = 200;
  const time_point_sec issue_time = db.head_block_time();

  // Error: no issued license.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_operation(vault_id, 100, 200, vs.as_string())), fc::exception );

  do_op(issue_license_operation(get_license_issuer_id(), vault_id, manager_locked.id, bonus_percent, frequency_lock, issue_time));

  // Error: issued other license.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_operation(vault_id, 100, 200, vs.as_string())), fc::exception );

  // Error: cannot submit zero cycles.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_operation(vault_id, 0, 200, vm.as_string())), fc::exception );

  // Error: not enough cycles on the balance.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_operation(vault_id, 2 * DASCOIN_BASE_MANAGER_CYCLES, 200, vm.as_string())), fc::exception );

  // Error: frequency (20) is not equal to license's frequency.
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_operation(vault_id, 1000, 20, vm.as_string())), fc::exception );

  do_op(submit_cycles_to_queue_operation(vault_id, 1000, 200, vm.as_string()));

  const auto& license_information_obj = (*vault.license_information)(db);
  const auto& license_history = license_information_obj.history;
  const auto& license_record = license_history[0];
  const uint32_t remaining_cycles = static_cast<uint32_t>(DASCOIN_BASE_MANAGER_CYCLES * 1.1) - 1000;
  BOOST_CHECK_EQUAL( license_record.amount.value, remaining_cycles );

  const auto& balance = get_cycle_balance(vault_id);
  BOOST_CHECK_EQUAL( balance.value, remaining_cycles );

  // Error: not enough cycles on the balance:
  GRAPHENE_REQUIRE_THROW( do_op(submit_cycles_to_queue_operation(vault_id, remaining_cycles + 1, 200, vm.as_string())), fc::exception );

  do_op(submit_cycles_to_queue_operation(vault_id, 1000, 200, vm.as_string()));

  const auto& license_information_obj2 = (*vault.license_information)(db);
  const auto& license_history2 = license_information_obj2.history;
  const auto& license_record2 = license_history2[0];
  BOOST_CHECK_EQUAL( license_record2.amount.value, remaining_cycles - 1000 );

  const auto& balance2 = get_cycle_balance(vault_id);
  BOOST_CHECK_EQUAL( balance2.value, remaining_cycles - 1000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( historic_sum_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  // No coins minted at the start:
  BOOST_CHECK_EQUAL(get_dynamic_global_properties().total_dascoin_minted.value, 0);

  // Submit four reserve batches of 100 Dascoin:
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 200, 200, ""));

  // Check historic sums for all elements:
  // Should be: minted = 0; first: 100; second 200; third 300; fourth 400;
  BOOST_CHECK_EQUAL(get_dynamic_global_properties().total_dascoin_minted.value, 0);
  auto queue = _dal.get_reward_queue();
  BOOST_CHECK_EQUAL(queue[0].historic_sum.value, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL(queue[1].historic_sum.value, 200 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL(queue[2].historic_sum.value, 300 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL(queue[3].historic_sum.value, 400 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Adjust reward to 100 coin:
  adjust_dascoin_reward(200 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the first distribution interval:
  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Check historic sums for all elements:
  // Should be: minted = 200; third 300; fourth 400;
  BOOST_CHECK_EQUAL(get_dynamic_global_properties().total_dascoin_minted.value, 200 * DASCOIN_DEFAULT_ASSET_PRECISION);
  queue = _dal.get_reward_queue();
  BOOST_CHECK_EQUAL(queue[0].historic_sum.value, 300 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL(queue[1].historic_sum.value, 400 * DASCOIN_DEFAULT_ASSET_PRECISION);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_time_on_queue_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  // Create the following queue:
  // Previuously minted: 400
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 800, 200, "test"));

  // 100 -> 200 -> 100 -> 300
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 100, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 400, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 600, 200, "test"));

  // Adjust reward to 100 coin:
  adjust_dascoin_reward(100 * DASCOIN_DEFAULT_ASSET_PRECISION);

  const auto& dgpo = get_dynamic_global_properties();
  const auto& gpo = get_global_properties();

  auto reward_amount = gpo.parameters.dascoin_reward_amount;
  auto reward_interval = gpo.parameters.reward_interval_time_seconds;

  // Wait for 4 intervals:
  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(reward_interval));
  generate_blocks(db.head_block_time() + fc::seconds(reward_interval));
  generate_blocks(db.head_block_time() + fc::seconds(reward_interval));
  generate_blocks(db.head_block_time() + fc::seconds(reward_interval));

  BOOST_CHECK_EQUAL(dgpo.total_dascoin_minted.value, 400 * DASCOIN_DEFAULT_ASSET_PRECISION);

  auto queue = _dal.get_reward_queue();
  vector<uint32_t> times;
  times.reserve(queue.size());
  for(const auto& el: queue)
    times.emplace_back(get_time_on_queue(el.historic_sum, dgpo.total_dascoin_minted, reward_amount, reward_interval));

  BOOST_CHECK_EQUAL(times[0], 0);
  BOOST_CHECK_EQUAL(times[1], 2 * reward_interval);
  BOOST_CHECK_EQUAL(times[2], 3 * reward_interval);
  BOOST_CHECK_EQUAL(times[3], 6 * reward_interval);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(submission_number_test)
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  BOOST_CHECK_EQUAL(db.get_dynamic_global_properties().last_minted_submission_num, 0);
  BOOST_CHECK_EQUAL(db.get_dynamic_global_properties().max_queue_submission_num, 0);

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 200, 200, "test"));

  // Now we have 4 submissions on the queue, 0 minted:
  auto last = db.get_dynamic_global_properties().last_minted_submission_num;
  auto max = db.get_dynamic_global_properties().max_queue_submission_num;
  BOOST_CHECK_EQUAL(last, 0);
  BOOST_CHECK_EQUAL(max, 4);
  BOOST_CHECK_EQUAL(_dal.get_reward_queue_size(), max-last);

  auto queue = _dal.get_reward_queue();
  BOOST_CHECK_EQUAL(queue[0].number, 1);
  BOOST_CHECK_EQUAL(queue[1].number, 2);
  BOOST_CHECK_EQUAL(queue[2].number, 3);
  BOOST_CHECK_EQUAL(queue[3].number, 4);

  // Mint two elements from the queue:
  adjust_dascoin_reward(100 * DASCOIN_DEFAULT_ASSET_PRECISION);
  toggle_reward_queue(true);
  auto reward_interval = get_global_properties().parameters.reward_interval_time_seconds;
  generate_blocks(db.head_block_time() + fc::seconds(reward_interval));
  generate_blocks(db.head_block_time() + fc::seconds(reward_interval));

  // Now we have 2 submissions on the queue, 2 minted:
  last = db.get_dynamic_global_properties().last_minted_submission_num;
  max = db.get_dynamic_global_properties().max_queue_submission_num;
  BOOST_CHECK_EQUAL(last, 2);
  BOOST_CHECK_EQUAL(max, 4);
  BOOST_CHECK_EQUAL(_dal.get_reward_queue_size(), max-last);

  queue = _dal.get_reward_queue();
  BOOST_CHECK_EQUAL(queue[0].number, 3);
  BOOST_CHECK_EQUAL(queue[1].number, 4);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(get_queue_by_page_test)
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200, "test"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 200, 200, "test"));

  // Now we have 4 submissions on the queue
  // Get the first queue entry
  auto queue = _dal.get_reward_queue_by_page(0, 1);
  BOOST_CHECK_EQUAL(queue.size(), 1);
  BOOST_CHECK_EQUAL(queue[0].number, 1);

  // Get first two entries from the queue
  queue = _dal.get_reward_queue_by_page(0, 2);
  BOOST_CHECK_EQUAL(queue.size(), 2);
  BOOST_CHECK_EQUAL(queue[0].number, 1);
  BOOST_CHECK_EQUAL(queue[1].number, 2);

  // Get the third entry
  queue = _dal.get_reward_queue_by_page(2, 1);
  BOOST_CHECK_EQUAL(queue.size(), 1);
  BOOST_CHECK_EQUAL(queue[0].number, 3);

  // Get from invalid position
  GRAPHENE_REQUIRE_THROW(_dal.get_reward_queue_by_page(4, 2), fc::exception);

  // Get invalid amount
  GRAPHENE_REQUIRE_THROW(_dal.get_reward_queue_by_page(1, 5), fc::exception);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
