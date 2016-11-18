/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/license_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( issue_web_asset_test )
{ try {
    ACTORS((alice)(bob))
    get_web_asset_id();
    const issue_asset_request_object* p_issue_asset_req = issue_webasset(alice_id, 100, 100);
    FC_ASSERT( p_issue_asset_req, "Asset request object for '${n}' was not created",("n",alice.name));
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
