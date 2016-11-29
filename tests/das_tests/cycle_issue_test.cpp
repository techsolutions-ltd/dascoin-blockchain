/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/protocol/license.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/cycle_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( license_types_create_test )
{ try {
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(vault);
  generate_block();

  const auto issue_cycle_request = [this](const account_object& account, share_type amount)
  {
    const cycle_issue_request_object* p = issue_cycles(account.id, amount);
    BOOST_CHECK( p );
    // BOOST_CHECK_EQUAL( p->cycle_issuer.number, get_cycle_issuer_id().number );
    // BOOST_CHECK_EQUAL( p->account.number, account.id.number );
    // BOOST_CHECK_EQUAL( p->amount.value, amount.value );
  };

  // Rejected, cannot issue to wallet directly:
  GRAPHENE_REQUIRE_THROW( issue_cycle_request(wallet, 1000), fc::exception );

  // Issue cycles to vault account:
  issue_cycle_request(vault, 1000);
  issue_cycle_request(vault, 2000);
  issue_cycle_request(vault, 3000);

  // There should be three request objects found:
  BOOST_CHECK_EQUAL( get_cycle_issue_request_objects_by_expiration(vault_id).size(), 3 );

  // Wait some time...
  generate_blocks(db.head_block_time() + fc::seconds(5));

  // Deny the first request:
  auto req = get_cycle_issue_request_objects_by_expiration(vault_id)[0];
  deny_issue_cycles(req.id);

  // Check if there are two requests in the queue
  BOOST_CHECK_EQUAL( get_cycle_issue_request_objects_by_expiration(vault_id).size(), 2 );

  // Wait for time to elapse:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Check if the second and third requests have been fulfilled:
  BOOST_CHECK_EQUAL( get_cycle_balance(vault_id).value, 5000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
