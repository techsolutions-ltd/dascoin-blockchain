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

BOOST_AUTO_TEST_CASE( min_max_transfer_limit_test )
{ try {
  auto chain_params = get_chain_parameters();

  BOOST_CHECK(chain_params.minimum_transfer_limit == DASCOIN_DEFAULT_MINIMUM_TRANSFER_LIMIT);
  BOOST_CHECK(chain_params.maximum_transfer_limit == DASCOIN_DEFAULT_MAXIMUM_TRANSFER_LIMIT);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( limit_enum_test )
{ try {

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
