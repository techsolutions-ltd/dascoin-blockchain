/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/wire_object.hpp>
#include <graphene/chain/protocol/wire.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_AUTO_TEST_CASE( wire_out_web_asset_test )
{ try {
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(vault);
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

  // Reject, insuficcient balance:
  GRAPHENE_REQUIRE_THROW( wire_out(wallet_id, asset(10000, get_web_asset_id())), fc::exception );

  issue_webasset(wallet_id, 15000, 15000);
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

  // Reject, daily limit breached:
  GRAPHENE_REQUIRE_THROW( wire_out(wallet_id, asset(10000, get_web_asset_id())), fc::exception );

  update_pi_limits(wallet_id, 99, {15000,15000,15000});
  wire_out(wallet_id, asset(10000, get_web_asset_id()));
  check_balances(wallet, 5000, 15000);

  // Check if the wire out object exists:


} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()
