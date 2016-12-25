/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( submit_user_cycles_test )
{ try {
  // (1) Prepare long term actors:
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(vault);
  generate_block();

  // (2) Attempt to submit from a vault -> reject
  GRAPHENE_CHECK_THROW( submit_cycles(vault_id, 100) , fc::exception );



} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
