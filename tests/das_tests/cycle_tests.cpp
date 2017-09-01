/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/access_layer.hpp>
#include <graphene/chain/exceptions.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )
BOOST_FIXTURE_TEST_SUITE( cycle_tests, database_fixture )

BOOST_AUTO_TEST_CASE( issue_free_cycles_test )
{ try {
  // VAULT_ACTOR(vault)

  // const auto ISSUER_ID = get_cycle_issuer_id();
  
  // // Issue cycles:
  // do_op(issue_free_cycles_operation(ISSUER_ID, cycle_origin_kind::upgrade, vault_id, 100, "TEST"));

  // // Check the cycle balance:
  // auto vault_balance = db.get_cycle_balance(vault_id);
  // BOOST_CHECK_EQUAL( vault_balance.value, 100 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::cycle_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
