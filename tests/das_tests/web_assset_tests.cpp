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
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(vault);
  generate_block();
  ACTOR(richguy);
  generate_block();

  const auto check_balances = [this](const account_object& account, share_type expected_cash,
                                     share_type expected_reserved)
  {
    share_type cash, reserved;
    std::tie(cash, reserved) = get_web_asset_amounts(account.id);
    bool amount_ok = (cash == expected_cash && reserved == expected_reserved);
    FC_ASSERT( amount_ok, "On account '${n}': balance = (${c}/${r}), expected = (${ec}/${er})",
              ("n", account.name)("c", cash)("r", reserved)("ec", expected_cash)("er", expected_reserved));
  };

  const auto issue_request = [this](const account_object& account, share_type cash, share_type reserved)
  {
    const issue_asset_request_object* p = issue_webasset(account.id, cash, reserved);
    FC_ASSERT( p, "Asset request object for '${n}' was not created",("n",account.name));
    // ilog( "Creating asset request for '${n}'", ("n", (p->receiver)(db).name));
    FC_ASSERT( p->issuer == get_webasset_issuer_id() );
    FC_ASSERT( p->receiver == account.id, "Receiver: '${n}', Account: '${an}'", ("n", (p->receiver)(db).name)("an", account.name));
    FC_ASSERT( p->amount == cash );
    FC_ASSERT( p->asset_id == get_web_asset_id() );
    FC_ASSERT( p->reserved_amount == reserved );
  };

  issue_request(wallet, 100, 100);
  issue_request(vault, 100, 100);
  issue_request(richguy, 600, 600);
  issue_request(richguy, 300, 300);
  issue_request(richguy, 100, 100);

  BOOST_CHECK_EQUAL( get_asset_request_objects(wallet_id).size(), 1);
  BOOST_CHECK_EQUAL( get_asset_request_objects(vault_id).size(), 1);
  BOOST_CHECK_EQUAL( get_asset_request_objects(richguy_id).size(), 3);

  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

  check_balances(wallet, 100, 100);
  check_balances(vault, 100, 100);
  check_balances(richguy, 1000, 1000);

  issue_request(richguy, 100, 0);
  BOOST_CHECK_EQUAL( get_asset_request_objects(richguy_id).size(), 1);
  deny_issue_request(get_asset_request_objects(richguy_id)[0].id);
  BOOST_CHECK_EQUAL( get_asset_request_objects(richguy_id).size(), 0);
  check_balances(richguy, 1000, 1000);

  // Transfer funds wallet --> vault:
  // Reject, arguments reversed:
  GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(vault_id, wallet_id, {50,50}), fc::exception );
  // Reject, accounts are not tethered:
  GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(wallet_id, vault_id, {50,50}), fc::exception );

  tether_accounts(wallet_id, vault_id);
  transfer_webasset_wallet_to_vault(wallet_id, vault_id, {50,50});
  check_balances(wallet, 50, 50);
  check_balances(vault, 150, 150);

  // Reject, not enough balance:
  GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(wallet_id, vault_id, {1000,1100}), fc::exception );

  // Update the limits to allow vault -> wallet transfer:
  // update_pi_limits(vault_id, 99, {100,100,100});

  // Reject, arguments reversed:
  GRAPHENE_REQUIRE_THROW( transfer_webasset_wallet_to_vault(wallet_id, vault_id, {100,100}), fc::exception );

  transfer_webasset_vault_to_wallet(vault_id, wallet_id, {75,75});
  check_balances(wallet, 125, 125);
  check_balances(vault, 75, 75);

  GRAPHENE_REQUIRE_THROW( transfer_webasset_vault_to_wallet(vault_id, wallet_id, {25,25}), fc::exception );


} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
