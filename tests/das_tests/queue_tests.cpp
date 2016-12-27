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
  // Prepare long term actors:
  ACTOR(wallet);
  VAULT_ACTOR(vault);
  VAULT_ACTOR(stan);

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

  // Attempt to submit from a wallet -> reject, cannot submit frow wallet
  // GRAPHENE_CHECK_THROW( submit_cycles(wallet_id, 100) , fc::exception );

  // Attempt to submit from empty vault -> reject, not enough cycles
  // GRAPHENE_CHECK_THROW( submit_cycles(vault_id, 100) , fc::exception );

  // Vault gets some cycles issued to it:
  issue_cycles(vault_id, 100);

  // Wait for the issue to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Check if the second and third requests have been fulfilled:
  BOOST_CHECK_EQUAL( get_cycle_balance(vault_id).value, 100 );

  // Submit from the vault:
  submit_cycles(vault_id, 100);

  // Issue license to Stan's vault account:
  issue(stan, "standard");

  // Wait for the license to process:
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Submit cycles from the license:
  submit_cycles(stan_id, 100);

  // (9) Check queue size, check element data:

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
