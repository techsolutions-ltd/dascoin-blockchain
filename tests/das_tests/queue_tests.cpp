/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/access_layer.hpp>
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

BOOST_AUTO_TEST_CASE( update_queue_parameters_unit_test )
{ try {

  do_op(update_queue_parameters_operation(get_license_issuer_id(), {true}, {600}, 
                                          {2000 * DASCOIN_DEFAULT_ASSET_PRECISION}));

  const auto& params = get_chain_parameters();
  BOOST_CHECK_EQUAL( params.enable_dascoin_queue, true );
  BOOST_CHECK_EQUAL( params.reward_interval_time_seconds, 600 );
  BOOST_CHECK_EQUAL( params.dascoin_reward_amount, 2000 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // TODO: handle negative cases

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_chartered_license_unit_test )
{ try {
  VAULT_ACTOR(first)

  auto lic_typ = *(_dal.get_license_type("standard-charter"));

  do_op(issue_license_operation(get_license_issuer_id(), first_id, lic_typ.id,
      10, 200, db.head_block_time()));

  auto pos_vec = _dal.get_queue_submissions_with_pos(first_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  uint32_t pos;
  reward_queue_object rqo;
  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 0 );

  BOOST_CHECK_EQUAL( rqo.origin, "charter_license" );
  BOOST_CHECK( rqo.license.valid() );
  BOOST_CHECK( *rqo.license == lic_typ.id );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 110 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( submit_cycles_unit_test )
{ try {
  VAULT_ACTOR(first)

  adjust_frequency(200);

  adjust_cycles(first_id, 200);
    
  do_op(submit_cycles_to_queue_operation(first_id, 100));

  auto pos_vec = _dal.get_queue_submissions_with_pos(first_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  uint32_t pos;
  reward_queue_object rqo;
  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 0 );

  BOOST_CHECK_EQUAL( rqo.origin, "user_submit" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 100 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

} FC_LOG_AND_RETHROW() }

/*BOOST_AUTO_TEST_CASE( get_queue_submissions_with_pos_unit_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 100, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 120, 200));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 200, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 210, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 220, 200));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 300, 200));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 400, 200));

  auto pos_vec = _dal.get_queue_submissions_with_pos(first_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 2 );

  uint32_t pos;
  reward_queue_object rqo;
  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 0 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 100 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );
  // BOOST_CHECK( rqo.time == db.head_block_time() ); // TODO: this fails for some reason?

  std::tie(pos, rqo) = pos_vec[1];
  BOOST_CHECK_EQUAL( pos, 1 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 120 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = _dal.get_queue_submissions_with_pos(second_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 3 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 2 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 200 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  std::tie(pos, rqo) = pos_vec[1];
  BOOST_CHECK_EQUAL( pos, 3 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 210 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  std::tie(pos, rqo) = pos_vec[2];
  BOOST_CHECK_EQUAL( pos, 4 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 220 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = _dal.get_queue_submissions_with_pos(third_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 5 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == third_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 300 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = _dal.get_queue_submissions_with_pos(fourth_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 6 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == fourth_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 400 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

} FC_LOG_AND_RETHROW() }*/

BOOST_AUTO_TEST_CASE( basic_submit_reserved_cycles_to_queue_test )
{ try {
  VAULT_ACTOR(first)
  VAULT_ACTOR(second)
  VAULT_ACTOR(third)
  VAULT_ACTOR(fourth)

  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 200, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 400, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 200, 200));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 600, 200));

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

  do_op(submit_cycles_to_queue_operation(first_id, 200));
  do_op(submit_cycles_to_queue_operation(second_id, 400));
  do_op(submit_cycles_to_queue_operation(third_id, 200));
  do_op(submit_cycles_to_queue_operation(fourth_id, 600));

  // Queue looks like this:
  // 200 --> 400 --> 200 --> 600

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 4 );

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

BOOST_AUTO_TEST_CASE( basic_chartered_license_to_queue_test )
{ try {

  const auto& issue =  [&](account_id_type account_id, const string& license_name, share_type bonus_percentage, 
                           share_type frequency_lock)
  {
    auto lic_typ = *(_dal.get_license_type(license_name));
    do_op(issue_license_operation(get_license_issuer_id(), account_id, lic_typ.id,
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

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 * DASCOIN_DEFAULT_ASSET_PRECISION );

  BOOST_CHECK_EQUAL( _dal.get_reward_queue_size(), 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
