/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>


#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/license_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( account_create_test )
{ try {
  ACTORS((alice)(charlie));
  VAULT_ACTORS((bob)(dude)(ethel));

  account_object test_obj;
  test_obj = get_account("alice");
  BOOST_CHECK( test_obj.is_wallet() );
  BOOST_CHECK( alice_id == test_obj.id );

  test_obj = get_account("bob");
  BOOST_CHECK( test_obj.is_vault() );
  BOOST_CHECK( bob_id == test_obj.id );

  test_obj = get_account("charlie");
  BOOST_CHECK( test_obj.is_wallet() );
  BOOST_CHECK( charlie_id == test_obj.id );

  test_obj = get_account("dude");
  BOOST_CHECK( test_obj.is_vault() );
  BOOST_CHECK( dude_id == test_obj.id );

} FC_LOG_AND_RETHROW() }

/*BOOST_AUTO_TEST_CASE( tether_accounts_test )
{ try {
  ACTORS((wallet1)(wallet2)(wallet3));
  VAULT_ACTORS((vault1)(vault2)(vault3));

  ilog("wallet1 name: '${n}' id: ${id}", ("n", wallet1.name)("id", wallet1_id));
  ilog("wallet2 name: '${n}' id: ${id}", ("n", wallet2.name)("id", wallet2_id));
  ilog("wallet3 name: '${n}' id: ${id}", ("n", wallet3.name)("id", wallet3_id));
  ilog("vault1 name: '${n}' id: ${id}", ("n", vault1.name)("id", vault1_id));
  ilog("vault2 name: '${n}' id: ${id}", ("n", vault1.name)("id", vault2_id));
  ilog("vault3 name: '${n}' id: ${id}", ("n", vault1.name)("id", vault3_id));

  // BOOST_TEST_CHECKPOINT("Tethering vault accounts 1, 2, 3 to wallet account 1:");
  // tether_accounts( wallet1_id, vault1_id );
  // tether_accounts( wallet1_id, vault2_id );
  // tether_accounts( wallet1_id, vault3_id );

  // BOOST_CHECK( wallet1.has_in_vault(vault1_id) );
  // BOOST_CHECK( wallet1.has_in_vault(vault2_id) );
  // BOOST_CHECK( wallet1.has_in_vault(vault3_id) );

  // BOOST_CHECK( vault1.has_in_parents(wallet1_id) );
  // BOOST_CHECK( vault2.has_in_parents(wallet1_id) );
  // BOOST_CHECK( vault3.has_in_parents(wallet1_id) );

  BOOST_TEST_CHECKPOINT("Attempting to tether wallet account 3 to wallet account 1:");
  GRAPHENE_REQUIRE_THROW( tether_accounts( wallet1_id, wallet3_id ), tether_accounts_no_vault_account );

  BOOST_TEST_CHECKPOINT("Attempting to tether vault account 2 to vault account 3:");
  GRAPHENE_REQUIRE_THROW( tether_accounts( vault3_id, vault2_id ), tether_accounts_no_wallet_account );

  // BOOST_TEST_CHECKPOINT("Tethering vault account 1 to wallet account 2, as the second parent");
  // tether_accounts( wallet2_id,  vault1_id );
  // BOOST_CHECK( wallet2.has_in_vault(vault1_id) );

} FC_LOG_AND_RETHROW() }*/

BOOST_AUTO_TEST_SUITE_END()
