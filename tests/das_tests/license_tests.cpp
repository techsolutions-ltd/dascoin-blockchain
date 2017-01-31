/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/protocol/license.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/license_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( upgrade_type_test )
{ try {

  upgrade_type test_pres_charter_upgrade({1,2,2});
  share_type x = 1000;

  // 1000 x1 = 1000
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 1000 );

  // 1000 x2 = 2000
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 2000 );

  // 2000 x2 = 4000
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 4000 );

  // After this it stays the same:
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 4000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( license_type_integrity_test )
{ try {
  // TODO: test for every type of license there is.

  auto lic_obj = get_license_type("standard");

  BOOST_CHECK_EQUAL( lic_obj.name, "standard" );
  BOOST_CHECK_EQUAL( lic_obj.amount.value, 100 );
  BOOST_CHECK_EQUAL( lic_obj.kind, license_kind::regular );
  BOOST_CHECK( lic_obj.balance_upgrade == upgrade_type({2}) );
  BOOST_CHECK( lic_obj.requeue_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.return_upgrade == upgrade_type() );

  lic_obj = get_license_type("standard-charter");

  BOOST_CHECK_EQUAL( lic_obj.name, "standard-charter" );
  BOOST_CHECK_EQUAL( lic_obj.amount.value, 100 );
  BOOST_CHECK_EQUAL( lic_obj.kind, license_kind::chartered );
  BOOST_CHECK( lic_obj.balance_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.requeue_upgrade == upgrade_type({1}) );
  BOOST_CHECK( lic_obj.return_upgrade == upgrade_type() );

  lic_obj = get_license_type("standard-promo");

  BOOST_CHECK_EQUAL( lic_obj.name, "standard-promo" );
  BOOST_CHECK_EQUAL( lic_obj.amount.value, 100 );
  BOOST_CHECK_EQUAL( lic_obj.kind, license_kind::promo );
  BOOST_CHECK( lic_obj.balance_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.requeue_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.return_upgrade == upgrade_type({1}) );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( edit_license_type_test )
{ try {
  using up_t = upgrade_multiplier_type;
  using o_up_t = optional<upgrade_multiplier_type>;
  
  auto empty = o_up_t(up_t());

  create_license_type("regular", "test", 100, {1, 2, 3}, {4, 5, 6}, {7, 8, 9});
  generate_block();
  auto lt = get_license_type("test");
  edit_license_type(lt.id, {"test-modified"}, {1000}, o_up_t(up_t{2, 4, 6}), empty, o_up_t());
  generate_block();
  
  lt = get_license_type("test-modified");
  BOOST_CHECK_EQUAL( lt.name, "test-modified" );
  BOOST_CHECK_EQUAL( lt.amount.value, 1000 );
  BOOST_CHECK_EQUAL( lt.kind, license_kind::regular );
  BOOST_CHECK( lt.balance_upgrade == upgrade_type({2, 4, 6}) );
  BOOST_CHECK( lt.requeue_upgrade == upgrade_type() );
  BOOST_CHECK( lt.return_upgrade == upgrade_type({7, 8, 9}) );


} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_license_test )
{ try {
  ACTOR(wallet);
  VAULT_ACTOR(stan);
  VAULT_ACTOR(allguy);

  const auto& issue = [&](const account_object& acc, const string& lic_name, frequency_type f = 0)
  { try {
    auto lic = get_license_type(lic_name);
    auto req = issue_license_to_vault_account(acc.id, lic.id);
    BOOST_CHECK( req );
    BOOST_CHECK( req->license_issuing_account == get_license_issuer_id() );
    BOOST_CHECK( req->account == acc.id );
    BOOST_CHECK( req->license == lic.id );
    BOOST_CHECK( req->frequency == f );
    generate_block();
  } FC_LOG_AND_RETHROW() };

  const auto& check_pending = [&](const account_object& acc, const string& lic_name)
  {
    auto lic = get_license_type(lic_name);
    auto pending = *(acc.license_info.pending);
    BOOST_CHECK( lic.id == pending.license );
  };

  const auto& generate_blocks_until_license_approved = [&]()
  {
    // Wait for license time to elapse:
    generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().license_expiration_time_seconds));
  };

  // Rejected: cannot issue to a vault account.
  GRAPHENE_REQUIRE_THROW( issue(wallet, "standard"), fc::exception );

  // Issue standard license to our old pal Stan, and Allguy:
  issue(stan, "standard");
  check_pending(stan, "standard");

  issue(allguy, "standard");
  check_pending(allguy, "standard");

  // Try and issue another license to stan:
  GRAPHENE_REQUIRE_THROW( issue(stan, "manager-charter"), fc::exception );

  generate_blocks_until_license_approved();

  // Check if Stan has 100 cycles:
  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 100 );

  // Now we try the Allguy:
  // Allguy should get all the licenses in order:
  license_information lic_info;
  issue(allguy, "manager");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2}) );

  issue(allguy, "pro");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 2600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2}) );

  issue(allguy, "executive");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 7600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2,2}) );

  issue(allguy, "president");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 32600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2,2,2}) );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_cycles_test )
{ try {
  VAULT_ACTOR(stan);
  generate_block();
  VAULT_ACTOR(richguy);
  generate_block();
  ACTOR(wallet);
  generate_block();

  const auto& issue = [&](const account_object& account, const string& license_name){
    issue_license_to_vault_account(account.id, get_license_type(license_name).id);
    generate_block();
  };

  issue(stan, "standard");  // 100 cycles.
  issue(richguy, "president");  // 25000 cycles.

  adjust_cycles(wallet_id, 2000);  // Wallet has no license, but has floating cycles.

  // Wait for time to elapse:
  // TODO: fetch the time parameter.
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Check balances on accounts:
  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 100 );  // 100
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 25000 );  // 25000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // 2000 (no license)

  // No upgrade happened yet!
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 0 );

  // Wait for the maintenace interval to trigger:
  generate_blocks(db.head_block_time() + fc::days(120));
  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);  // Just in case, on the maintenance int.

  // One upgrade event has happened:
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 1 );

  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 200 );  // 100 -> 200
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 50000 );  // 25000 -> 50000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // Wallet should get no increase.

  // Wait for the maintenace interval to trigger:
  generate_blocks(db.head_block_time() + fc::days(120));
  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  // Second upgrade event has happened:
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 2 );

  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 200 );
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 100000 );  // 50000 -> 100000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // No increase.

  // Wait for the maintenace interval to trigger:
  generate_blocks(db.head_block_time() + fc::days(120));
  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  // Second upgrade event has happened:
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 3 );

  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 200 );
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 200000 );  //  100000 -> 200000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // No increase.

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_single_license_test )
{ try {

  VAULT_ACTOR(vault);
  const auto pro_id = get_license_type("pro").id;

  issue_license_to_vault_account(vault_id, pro_id);
  generate_block();

  generate_blocks(db.head_block_time() + fc::hours(24));
  BOOST_CHECK( vault.license_info.active_license() == pro_id );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
