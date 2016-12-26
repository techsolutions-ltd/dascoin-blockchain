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

  // (2) Attempt to submit from a wallet -> reject, no balance
  GRAPHENE_CHECK_THROW( submit_cycles(wallet_id, 100) , fc::exception );

  // (3) Wallet gets some cycles issued to it:
  issue_cycles(wallet_id, 100);

  // (4) Wait for the issue to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // (5) Submit from the wallet:
  submit_cycles(wallet_id, 100);

  // (6) Issue license to vault account:
  issue_license_to_vault_account(vault_id, get_license_type("standard").id);

  // (7) Wait for the license to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // (8) Submit cycles from the license:
  submit_cycles(vault_id, 100);

  // (9) Check queue size, check element data:



} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
