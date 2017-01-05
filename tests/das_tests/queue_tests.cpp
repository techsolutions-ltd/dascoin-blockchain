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

BOOST_AUTO_TEST_CASE( submit_user_cycles_test )
{ try {
  // Prepare long term actors:
  ACTOR(wallet);
  VAULT_ACTOR(vault);
  VAULT_ACTOR(stan);
  VAULT_ACTOR(coolguy);
  VAULT_ACTOR(promoguy);

  const auto& issue = [&](const account_object& acc, const string& lic_name, frequency_type f = 0){
    auto lic = get_license_type(lic_name);
    auto req = issue_license_to_vault_account(acc.id, lic.id);
    BOOST_CHECK( req );
    BOOST_CHECK( req->license_issuing_account == get_license_issuer_id() );
    BOOST_CHECK( req->account == acc.id );
    BOOST_CHECK( req->license == lic.id );
    BOOST_CHECK( req->frequency == f );
    generate_block();
  };

  const auto& check = [&](const reward_queue_object& rqo, const account_object& acc, share_type am, frequency_type f){
    BOOST_CHECK( rqo.account == acc.id );
    BOOST_CHECK_EQUAL( rqo.amount.value, am.value );
    BOOST_CHECK_EQUAL( rqo.frequency.value, f.value );
  };

  // Attempt to submit from a wallet -> reject, cannot submit frow wallet
  GRAPHENE_CHECK_THROW( submit_cycles(wallet_id, 100) , fc::exception );

  // Attempt to submit from empty vault -> reject, not enough cycles
  GRAPHENE_CHECK_THROW( submit_cycles(vault_id, 100) , fc::exception );

  // Issue chartered license to the cool person:
  issue(coolguy, "pro-charter");

  // Wait for the issue to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // It should correspond to the pro license:
  check(get_reward_queue_objects_by_account(coolguy_id)[0], coolguy, 2000, coolguy.license_info.active_frequency_lock());

  // // Issue promo license to the promo person:
  issue(promoguy, "executive-promo");

  // // Wait for the issue to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // // It should correspond to the pro license:
  check(get_reward_queue_objects_by_account(promoguy_id)[0], promoguy, 5000, promoguy.license_info.active_frequency_lock());

  // Vault gets some cycles issued to it:
  issue_cycles(vault_id, 500);

  // Wait for the issue to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Check if the second and third requests have been fulfilled:
  BOOST_CHECK_EQUAL( get_cycle_balance(vault_id).value, 500 );

  // Submit from the vault:
  submit_cycles(vault_id, 500);

  // Issue license to Stan's vault account:
  issue(stan, "standard");

  // Wait for the license to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Submit cycles from the license:
  submit_cycles(stan_id, 100);

  check(get_reward_queue_objects_by_account(vault_id)[0], vault, 500, 0);
  check(get_reward_queue_objects_by_account(stan_id)[0], stan, 100, 0);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( basic_queue_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  adjust_dascoin_reward(500);
  adjust_frequency(200);  // Precision is 100 so this is 2.0.
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

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Dascoin amounts shoud be:
  // 100, 200, 100, 100

  BOOST_CHECK_EQUAL( get_balance(first_id, get_dascoin_asset_id()), 100 );
  BOOST_CHECK_EQUAL( get_balance(second_id, get_dascoin_asset_id()), 200 );
  BOOST_CHECK_EQUAL( get_balance(third_id, get_dascoin_asset_id()), 100 );
  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 100 );

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  BOOST_CHECK_EQUAL( get_balance(fourth_id, get_dascoin_asset_id()), 300 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
