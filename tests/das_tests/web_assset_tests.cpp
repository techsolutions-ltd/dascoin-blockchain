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

BOOST_AUTO_TEST_CASE( web_asset_test )
{ try {
  ACTORS((wallet))
  VAULT_ACTORS((vault))

  const auto check_cash_reserved = [this](account_id_type account_id, share_type expected_cash,
                                          share_type expected_reserved) {
    share_type cash, reserved;
    std::tie(cash, reserved) = get_web_asset_amounts(account_id);
    FC_ASSERT( cash == expected_cash, "cash = ${c}, expected = ${e}", ("c", cash)("e", expected_cash));
    FC_ASSERT( reserved == expected_reserved, "reserved = ${r}, expected = ${r}", ("r", reserved)("e", expected_reserved) );
  };

  const issue_asset_request_object* p_issue_asset_req = issue_webasset(wallet_id, 100, 100);
  FC_ASSERT( p_issue_asset_req, "Asset request object for '${n}' was not created",("n",wallet.name));

  BOOST_CHECK( p_issue_asset_req->issuer == get_webasset_issuer_id() );
  BOOST_CHECK( p_issue_asset_req->receiver == wallet_id );
  BOOST_CHECK( p_issue_asset_req->amount == asset(100, get_web_asset_id()) );
  BOOST_CHECK( p_issue_asset_req->reserved_amount == 100 );

  p_issue_asset_req = issue_webasset(vault_id, 100, 100);
  FC_ASSERT( p_issue_asset_req, "Asset request object for '${n}' was not created",("n",vault.name));

  BOOST_CHECK( p_issue_asset_req->issuer == get_webasset_issuer_id() );
  BOOST_CHECK( p_issue_asset_req->receiver == vault_id );
  BOOST_CHECK( p_issue_asset_req->amount == asset(100, get_web_asset_id()) );
  BOOST_CHECK( p_issue_asset_req->reserved_amount == 100 );

  generate_blocks(fc::time_point::now() + fc::minutes(10));

  check_cash_reserved(wallet_id, 100, 100);
  check_cash_reserved(vault_id, 100, 100);

  // Transfer funds wallet --> vault:
  // Arguments reversed:
  GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(vault_id, wallet_id, {50,50}), fc::exception );
  // Accounts are not tethered!
  GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(wallet_id, vault_id, {50,50}), fc::exception );

  tether_accounts(wallet_id, vault_id);
  transfer_webasset_wallet_to_vault(wallet_id, vault_id, {50,50});
  check_cash_reserved(vault_id, 150, 150);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
