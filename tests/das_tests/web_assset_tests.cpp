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
  ACTORS((wallet)(richguy))
  VAULT_ACTORS((vault))
  generate_block();

  const auto check_cash_reserved = [this](const account_object& account, share_type expected_cash,
                                          share_type expected_reserved) {
    share_type cash, reserved;
    std::tie(cash, reserved) = get_web_asset_amounts(account.id);
    bool amount_ok = (cash == expected_cash && reserved == expected_reserved);
    FC_ASSERT( amount_ok, "On account '${n}': balance = (${c}/${r}), expected = (${ec}/${er})",
              ("n", account.name)("c", cash)("r", reserved)("ec", expected_cash)("er", expected_reserved));
  };

  const auto issue_request = [this](const account_object& account, share_type cash, share_type reserved) {
    const issue_asset_request_object* p = issue_webasset(account.id, cash, reserved);
    FC_ASSERT( p, "Asset request object for '${n}' was not created",("n",account.name));
    BOOST_CHECK( p->issuer == get_webasset_issuer_id() );
    BOOST_CHECK( p->receiver == account.id );
    BOOST_CHECK( p->amount == cash );
    BOOST_CHECK( p->asset_id == get_web_asset_id() );
    BOOST_CHECK( p->reserved_amount == reserved );
  };

  issue_request(wallet, 100, 100);
  issue_request(vault, 100, 100);
  issue_request(richguy, 600, 600);
  issue_request(richguy, 300, 300);
  issue_request(richguy, 100, 100);

  generate_block();
  generate_blocks(db.head_block_time() + fc::minutes(10));

  check_cash_reserved(wallet, 100, 100);
  check_cash_reserved(vault, 100, 100);
  check_cash_reserved(richguy, 1000, 1000);

  issue_request(richguy, 100, 100);

  generate_blocks(fc::time_point::now() + fc::minutes(1));

  // // deny_issue_request(req_id);

  generate_blocks(fc::time_point::now() + fc::minutes(1));

  // // TODO: check if the request object exists.

  // check_cash_reserved(wallet_id, 100, 100);
  // check_cash_reserved(vault_id, 100, 100);
  // check_cash_reserved(richguy_id, 1000, 1000);

  // // Transfer funds wallet --> vault:
  // // Arguments reversed:
  // GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(vault_id, wallet_id, {50,50}), fc::exception );
  // // Accounts are not tethered!
  // GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(wallet_id, vault_id, {50,50}), fc::exception );

  // tether_accounts(wallet_id, vault_id);
  // transfer_webasset_wallet_to_vault(wallet_id, vault_id, {50,50});
  // check_cash_reserved(vault_id, 150, 150);

  // Reject, not enough balance:
  // GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(wallet_id, vault_id, {1000,1100}), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
