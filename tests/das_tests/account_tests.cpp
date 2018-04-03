/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( account_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( get_free_cycle_balance_non_existant_id_unit_test )
{ try {
  VAULT_ACTOR(vault);

  const fc::ecc::private_key new_key = fc::ecc::private_key::generate();
  const public_key_type key_id = new_key.get_public_key();
  const auto owner = authority(2, key_id, 1, init_account_pub_key, 1);
  const auto active = authority(2, key_id, 1, init_account_pub_key, 1);

  do_op(change_public_keys_operation(vault_id, {owner}, {active}));

  BOOST_TEST_MESSAGE("Updating vault account private keys.");

  BOOST_CHECK(vault.active.weight_threshold == 2);
  BOOST_CHECK(vault.active.num_auths() == 2);
  BOOST_CHECK(vault.active.key_auths.at(key_id) == 1);
  BOOST_CHECK(vault.active.key_auths.at(init_account_pub_key) == 1);
  BOOST_CHECK(vault.active_change_counter == 1);

  BOOST_CHECK(vault.owner.weight_threshold == 2);
  BOOST_CHECK(vault.owner.num_auths() == 2);
  BOOST_CHECK(vault.owner.key_auths.at(key_id) == 1);
  BOOST_CHECK(vault.owner.key_auths.at(init_account_pub_key) == 1);
  BOOST_CHECK(vault.owner_change_counter == 1);

  do_op(change_public_keys_operation(vault_id, {owner}, {active}));

  // Keys are changed twice at this point:
  BOOST_CHECK(vault.active_change_counter == 2);
  BOOST_CHECK(vault.owner_change_counter == 2);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(starting_amount_of_cycle_asset_test)
{
  try
  {
    // Create some accounts and check if they have default balance
    ACTOR(wallet0)
    ACTOR(wallet1)
    VAULT_ACTOR(vault0);

    // Wallet accounts should have default balance
    auto balance = get_balance(wallet0_id, get_cycle_asset_id());
    BOOST_CHECK_EQUAL(balance, DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT);
    balance = get_balance(wallet1_id, get_cycle_asset_id());
    BOOST_CHECK_EQUAL(balance, DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT);
    // Vault account should have empty balance
    balance = get_balance(vault0_id, get_cycle_asset_id());
    BOOST_CHECK_EQUAL(balance, 0);

    // Change starting balance
    auto root_id = db.get_global_properties().authorities.root_administrator;
    do_op(set_starting_cycle_asset_amount_operation(root_id, 145));

    // Make sure it works for wallets ...
    ACTOR(wallet2)
    balance = get_balance(wallet2_id, get_cycle_asset_id());
    BOOST_CHECK_EQUAL(balance, 145);
    // ... and that nothing is change for vaults
    VAULT_ACTOR(vault1);
    balance = get_balance(vault1_id, get_cycle_asset_id());
    BOOST_CHECK_EQUAL(balance, 0);
  }
  FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()  // account_unit_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
