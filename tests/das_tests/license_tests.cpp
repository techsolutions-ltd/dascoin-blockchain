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

// BOOST_AUTO_TEST_CASE( license_types_create_test )
// { try {

//   auto create_test_license = [&](const string& name, share_type amount, uint8_t upgrades, bool is_chartered = false)
//   {
//     const auto ret_license = create_license_type( name, amount, upgrades, is_chartered );
//     FC_ASSERT( ret_license, "License object ${n} does not exist!", ("n",name) );
//     BOOST_CHECK_EQUAL( ret_license->name, name );
//     BOOST_CHECK_EQUAL( ret_license->amount.value, amount.value );
//     BOOST_CHECK_EQUAL( ret_license->upgrades, upgrades );
//     ilog( "Created license ${n} with value ${v} and ${u} upgrades", ("n",name)("v",amount)("u",upgrades) );
//     if (is_chartered)
//       BOOST_CHECK( ret_license->is_chartered() );

//     get_license_type(name);
//     return ret_license;
//   };

//   create_test_license("test-low", 100, 1);
//   create_test_license("test-medium", 100, 5);
//   create_test_license("test-high", 1000, 10);

//   create_test_license("test-charter-low", 100, 1, true);
//   create_test_license("test-charter-medium", 100, 5, true);
//   create_test_license("test-charter-high", 1000, 10, true);

// } FC_LOG_AND_RETHROW() }

// BOOST_AUTO_TEST_CASE( issue_license_test )
// { try {
//   // issue_license_to_vault_account()
// } FC_LOG_AND_RETHROW() }

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
