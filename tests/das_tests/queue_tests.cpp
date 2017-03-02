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
  generate_block();

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 200, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 400, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 600, 200));

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

  const auto& issue =  [&](account_id_type account_id, const string& license_name, share_type bonus_percentage, 
                           share_type frequency_lock)
  {
    do_op(issue_license_operation(get_license_issuer_id(), account_id, get_license_type(license_name).id,
        bonus_percentage, frequency_lock, db.head_block_time()));
  };

  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  issue(first_id, "standard-charter", 100, 200);  // 100 + 1 * 100 = 200 cycles
  issue(second_id, "standard-charter", 300, 200);  // 100 + 3 * 100 = 400 cycles
  issue(third_id, "standard-charter", 100, 200);  // 100 + 1 * 100 = 200 cycles
  issue(fourth_id, "standard-charter", 500, 200);  // 100 + 5 * 100 = 600 cycles

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
