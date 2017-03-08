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

BOOST_FIXTURE_TEST_SUITE( license_tests, database_fixture )

BOOST_AUTO_TEST_CASE( regression_test_license_information_index )
{ try {

  db.create<license_information_object>([&](license_information_object& lio){});
  db.create<license_information_object>([&](license_information_object& lio){});

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( license_information_unit_test )
{ try {
  VAULT_ACTOR(vault);

  time_point_sec issue_time = db.head_block_time();
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, get_license_type("standard-charter").id,
        50, 200, issue_time));

  BOOST_CHECK( vault.license_information.valid() );

  const auto& license_information_obj = (*vault.license_information)(db);

  BOOST_CHECK( license_information_obj.account == vault_id );
  
  const auto& license_history = license_information_obj.history;

  BOOST_CHECK_EQUAL( license_history.size(), 1 );

  const auto& license_record = license_history[0];

  BOOST_CHECK( license_record.license == get_license_type("standard-charter").id );
  BOOST_CHECK_EQUAL( license_record.amount.value, 150 );
  BOOST_CHECK_EQUAL( license_record.base_amount.value, 100 );
  BOOST_CHECK_EQUAL( license_record.bonus_percent.value, 50 );
  BOOST_CHECK_EQUAL( license_record.frequency_lock.value, 200 );
  BOOST_CHECK( license_record.activated_at == issue_time );
  BOOST_CHECK( license_record.issued_on_blockchain == issue_time );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_license_types_unit_test )
{ try {

  auto lic_vec = get_license_types();

  BOOST_CHECK_EQUAL( lic_vec.size(), 18 );
  
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

/*BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( license_tests, database_fixture )

BOOST_AUTO_TEST_CASE( issue_single_license_test )
{ try {
  VAULT_ACTOR(vault);
  const auto pro_id = get_license_type("pro").id;

  issue_license_to_vault_account(vault_id, pro_id, 0, 0);
  generate_blocks_until_license_approved();

  BOOST_CHECK( vault.license_info.max_license() == pro_id );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_license_with_bonus_cycles )
{ try {
  const frequency_type frequency_lock = 200;
  VAULT_ACTOR(v100);
  VAULT_ACTOR(v150);
  VAULT_ACTOR(v50);
  VAULT_ACTOR(vzero);
  VAULT_ACTOR(vneg)
  vector<license_request_object> requests;

  const auto& get_pending_request = [&](const account_object& account) -> const license_request_object&
  {
    return (*account.license_info.pending).request(db);
  };
  
  issue_license_to_vault_account(v100, "standard-charter", 0, frequency_lock);
  issue_license_to_vault_account(v150, "standard-charter", 50, frequency_lock);
  issue_license_to_vault_account(v50, "standard-charter", -50, frequency_lock);
  GRAPHENE_CHECK_THROW( issue_license_to_vault_account(vzero, "standard-charter", -100, frequency_lock), fc::exception );
  GRAPHENE_CHECK_THROW( issue_license_to_vault_account(vneg, "standard-charter", -200, frequency_lock), fc::exception );

  BOOST_CHECK_EQUAL( get_pending_request(v100).amount.value, 100 );
  BOOST_CHECK_EQUAL( get_pending_request(v150).amount.value, 150 );
  BOOST_CHECK_EQUAL( get_pending_request(v50).amount.value, 50 );

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

BOOST_AUTO_TEST_CASE( check_issue_frequency_lock_not_zero )
{ try {
  VAULT_ACTOR(vault);

  // Regular license can have a frequency lock of 0:
  issue_license_to_vault_account(vault_id, get_license_type("pro").id, 0, 0);

  // Charter license CANNOT have a frequency lock of 0:
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(vault_id, get_license_type("pro-charter").id, 0, 0), fc::exception );

  // Promo license CANNOT have a frequency lock of 0:
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(vault_id, get_license_type("pro-promo").id, 0, 0), fc::exception );

} FC_LOG_AND_RETHROW() }

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

BOOST_AUTO_TEST_CASE( issue_license_test )
{ try {
  ACTOR(wallet);
  VAULT_ACTOR(stan);
  VAULT_ACTOR(allguy);

  const auto& check_pending = [&](const account_object& acc, const string& lic_name)
  {
    auto lic = get_license_type(lic_name);
    auto pending = *(acc.license_info.pending);
    BOOST_CHECK( lic.id == pending.license );
  };

  // Rejected: cannot issue to a vault account.
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(wallet, "standard"), fc::exception );

  // Issue standard license to our old pal Stan, and Allguy:
  issue_license_to_vault_account(stan, "standard");
  check_pending(stan, "standard");

  issue_license_to_vault_account(allguy, "standard");
  check_pending(allguy, "standard");

  // Try and issue another license to stan:
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(stan, "manager-charter", 0, 200), fc::exception );

  generate_blocks_until_license_approved();

  // Check if Stan has 100 cycles:
  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 100 );

  // Now we try the Allguy:
  // Allguy should get all the licenses in order:
  license_information lic_info;
  issue_license_to_vault_account(allguy, "manager");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2}) );

  issue_license_to_vault_account(allguy, "pro");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 2600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2}) );

  issue_license_to_vault_account(allguy, "executive");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 7600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2,2}) );

  issue_license_to_vault_account(allguy, "president");
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

  issue_license_to_vault_account(stan, "standard");  // 100 cycles.
  issue_license_to_vault_account(richguy, "president");  // 25000 cycles.

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

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()*/
