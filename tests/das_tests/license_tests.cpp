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

BOOST_AUTO_TEST_CASE( issue_license_test )
{ try {
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(stan);
  generate_block();
  VAULT_ACTOR(mccool);
  generate_block();

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

  // Rejected: cannot issue to a vault account.
  GRAPHENE_REQUIRE_THROW( issue(wallet, "standard"), fc::exception );

  // Issue standard license to our old pal Stan:
  issue(stan, "standard");

  // Issue a bunch of licenses to mccool:
  issue(mccool, "manager-charter");
  GRAPHENE_REQUIRE_THROW( issue(mccool, "standard"), fc::exception );
  issue(mccool, "pro");
  GRAPHENE_REQUIRE_THROW( issue(mccool, "pro-charter"), fc::exception );
  issue(mccool, "president-promo");
  GRAPHENE_REQUIRE_THROW( issue(mccool, "president"), fc::exception );
  // Mccool should get the president-promo license.

  // Wait for time to elapse:
  // TODO: fetch the time parameter
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Check if Stan has 100 cycles:
  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 100 );
  // Check the license history:
  auto history_vec = get_license_history(mccool_id);
  BOOST_CHECK_EQUAL(history_vec.size(), 3);
  BOOST_CHECK_EQUAL(history_vec[0].name, "manager-charter");
  BOOST_CHECK_EQUAL(history_vec[1].name, "pro");
  BOOST_CHECK_EQUAL(history_vec[2].name, "president-promo");

  // Mccool should have the following history:
  // manager charter license -> regular pro -> president promo
  // As such his account should have:
  // 1) 2000 cycles in the cycle balance from the promo
  // 2) upgrade_type({1}) on requeue
  // 3) upgrade_type({1,2,4}) on return
  BOOST_CHECK_EQUAL( get_cycle_balance(mccool_id).value, 2000 );

  const auto& lic_info = mccool_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type() );
  BOOST_CHECK( lic_info.requeue_upgrade == upgrade_type({1}) );
  BOOST_CHECK( lic_info.return_upgrade == upgrade_type({1,2,4}) );

} FC_LOG_AND_RETHROW() }

// BOOST_AUTO_TEST_CASE( upgrade_cycles_test )
// { try {
//   VAULT_ACTORS((holder));
//   INVOKE(license_types_create_test);
//   auto& lt = get_license_type("test-medium");

//   issue_license_to_vault_account(get_license_issuer_id(), holder_id, lt.id, {});
//   generate_blocks(fc::time_point::now() + fc::minutes(10));
//   BOOST_CHECK(get_cycle_balance(holder_id) == 100);

//   generate_blocks(fc::time_point::now() + fc::minutes(60));
//   BOOST_CHECK(get_cycle_balance(holder_id) == 200);

// } FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
