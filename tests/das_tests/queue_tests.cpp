/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/queue_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( queue_tests, database_fixture )

BOOST_AUTO_TEST_CASE( convert_dascoin_cycles_test )
{
  share_type amount = db.cycles_to_dascoin(10000, 300);
  BOOST_CHECK_EQUAL( amount.value, 33333333 );

  amount = db.dascoin_to_cycles(33333333, 300);
  BOOST_CHECK_EQUAL( amount.value, 9999 );
}

BOOST_AUTO_TEST_CASE( basic_submit_reserved_cycles_to_queue_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  submit_reserve_cycles_to_queue(first_id, 200, 200);
  submit_reserve_cycles_to_queue(second_id, 400, 200);
  submit_reserve_cycles_to_queue(third_id, 200, 200);
  submit_reserve_cycles_to_queue(fourth_id, 600, 200);

  // Wait for requests to pass:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().cycle_request_expiration_time_seconds));

  // Queue looks like this:
  // 200 --> 400 --> 200 --> 600

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 4 );

  toggle_reward_queue(true);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 100, 200, 100, 100

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 1 );

  // Wait for the rest if the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( basic_submit_cycles_to_queue_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);
  toggle_reward_queue(true);

  adjust_cycles(first_id, 200);
  adjust_cycles(second_id, 400);
  adjust_cycles(third_id, 200);
  adjust_cycles(fourth_id, 600);

  submit_cycles(first_id, 200);
  submit_cycles(second_id, 400);
  submit_cycles(third_id, 200);
  submit_cycles(fourth_id, 600);

  // Queue looks like this:
  // 200 --> 400 --> 200 --> 600

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 4 );

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 100, 200, 100, 100

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 1 );

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( basic_chartered_license_to_queue_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  auto standard_id = get_license_type("standard-charter").id;  // base = 100 cycles.

  issue_license_to_vault_account(first_id, standard_id, 100, 200);  // 100 + 1 * 100 = 200 cycles
  generate_block();
  issue_license_to_vault_account(second_id, standard_id, 300, 200);  // 100 + 3 * 100 = 400 cycles
  generate_block();
  issue_license_to_vault_account(third_id, standard_id, 100, 200);  // 100 + 1 * 100 = 200 cycles
  generate_block();
  issue_license_to_vault_account(fourth_id, standard_id, 500, 200);  // 100 + 5 * 100 = 600 cycles
  generate_block();

  // Queue looks like this:
  // 200 --> 400 --> 200 --> 600

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 4 );

  toggle_reward_queue(true);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 100, 200, 100, 100

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 1 );

  // Wait for the rest if the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
