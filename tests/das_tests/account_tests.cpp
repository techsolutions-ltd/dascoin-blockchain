/*
 * MIT License
 *
 * Copyright (c) 2018 Tech Solutions Malta LTD
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>

#include <graphene/chain/account_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( account_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( roll_back_account_unit_test )
{ try {
  ACTOR(wallet);

  BOOST_CHECK(wallet.owner == wallet.owner_roll_back && wallet.active == wallet.active_roll_back);
  BOOST_CHECK(!wallet.roll_back_active && wallet.roll_back_enabled);

  const fc::ecc::private_key priv_key_w1 = fc::ecc::private_key::generate();
  const public_key_type pub_key_w1 = priv_key_w1.get_public_key();
  const auto owner_w1 = authority(2, pub_key_w1, 1, init_account_pub_key, 1);
  const auto active_w1 = authority(2, pub_key_w1, 1, init_account_pub_key, 1);

  BOOST_TEST_MESSAGE("Change wallet account public keys.");
  do_op(change_public_keys_operation(wallet_id, {owner_w1}, {active_w1}));
  BOOST_CHECK(wallet.active_change_counter == 1);
  BOOST_CHECK(wallet.owner_change_counter == 1);

  BOOST_TEST_MESSAGE("Opt-out from public key roll back.");
  do_op(set_roll_back_enabled_operation(wallet_id, false));
  GRAPHENE_REQUIRE_THROW(do_op(roll_back_public_keys_operation(get_pi_validator_id(), wallet_id)), fc::exception);

  BOOST_TEST_MESSAGE("Opt-in to public key roll back.");
  do_op(set_roll_back_enabled_operation(wallet_id, true));

  BOOST_TEST_MESSAGE("Roll back public keys for account.");
  do_op(roll_back_public_keys_operation(get_pi_validator_id(), wallet_id));

  BOOST_TEST_MESSAGE("Roll back to puppet keys.");
  BOOST_CHECK(wallet.owner == wallet.owner_roll_back && wallet.active == wallet.active_roll_back);

  issue_webasset("1", wallet_id, 15000, 15000);
  GRAPHENE_REQUIRE_THROW(wire_out_with_fee(wallet_id, web_asset(10000), "BTC", "SOME_BTC_ADDRESS", "debit"), fc::exception);

  const fc::ecc::private_key priv_key_w2 = fc::ecc::private_key::generate();
  const public_key_type pub_key_w2 = priv_key_w2.get_public_key();
  const auto owner_w2 = authority(2, pub_key_w2, 1, init_account_pub_key, 1);
  const auto active_w2 = authority(2, pub_key_w2, 1, init_account_pub_key, 1);

  BOOST_TEST_MESSAGE("Change wallet account public keys to new ones.");
  do_op(change_public_keys_operation(wallet_id, {owner_w2}, {active_w2}));
  BOOST_CHECK(wallet.active_change_counter == 2);
  BOOST_CHECK(wallet.owner_change_counter == 2);

  wire_out_with_fee(wallet_id, web_asset(10000), "BTC", "SOME_BTC_ADDRESS", "debit");

  // Similar test for vault account
  VAULT_ACTOR(vault);
  BOOST_CHECK(vault.owner == vault.owner_roll_back && vault.active == vault.active_roll_back);
  BOOST_CHECK(!vault.roll_back_active && vault.roll_back_enabled);

  const fc::ecc::private_key priv_key_v1 = fc::ecc::private_key::generate();
  const public_key_type pub_key_v1 = priv_key_v1.get_public_key();
  const auto owner_v1 = authority(2, pub_key_v1, 1, init_account_pub_key, 1);
  const auto active_v1 = authority(2, pub_key_v1, 1, init_account_pub_key, 1);

  BOOST_TEST_MESSAGE("Change vault account public keys.");
  do_op(change_public_keys_operation(vault_id, {owner_v1}, {active_v1}));
  BOOST_CHECK(vault.active_change_counter == 1);
  BOOST_CHECK(vault.owner_change_counter == 1);

  BOOST_TEST_MESSAGE("Opt-out from public key roll back.");
  do_op(set_roll_back_enabled_operation(vault_id, false));
  GRAPHENE_REQUIRE_THROW(do_op(roll_back_public_keys_operation(get_pi_validator_id(), vault_id)), fc::exception);

  BOOST_TEST_MESSAGE("Opt-in to public key roll back.");
  do_op(set_roll_back_enabled_operation(vault_id, true));

  BOOST_TEST_MESSAGE("Roll back public keys for account.");
  do_op(roll_back_public_keys_operation(get_pi_validator_id(), vault_id));

  BOOST_TEST_MESSAGE("Roll back to puppet keys.");
  BOOST_CHECK(vault.owner == vault.owner_roll_back && vault.active == vault.active_roll_back);

  GRAPHENE_REQUIRE_THROW(tether_accounts(wallet_id, vault_id), fc::exception);

  const fc::ecc::private_key priv_key2 = fc::ecc::private_key::generate();
  const public_key_type pub_key2 = priv_key2.get_public_key();
  const auto owner2 = authority(2, pub_key2, 1, init_account_pub_key, 1);
  const auto active2 = authority(2, pub_key2, 1, init_account_pub_key, 1);

  BOOST_TEST_MESSAGE("Change vault account public keys to new ones.");
  do_op(change_public_keys_operation(vault_id, {owner2}, {active2}));
  BOOST_CHECK(vault.active_change_counter == 2);
  BOOST_CHECK(vault.owner_change_counter == 2);

  auto executive_locked = *(_dal.get_license_type("executive_locked"));
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, executive_locked.id, 50, 20, db.head_block_time()));
  do_op(submit_cycles_to_queue_by_license_operation(vault_id, 1000, executive_locked.id, 20, "TEST"));

} FC_LOG_AND_RETHROW() }

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

// this test works only when HARDFORK_EXEX_102_TIME is 1509516032
// if we change this time test will fail and we need to rewrite test
// to work with new time, basically we have to generate blocks to that moment....
BOOST_AUTO_TEST_CASE( remove_limit_from_all_vaults_test )
{ try {

  VAULT_ACTOR(vault1);

  BOOST_CHECK(!vault1.disable_vault_to_wallet_limit);

  generate_blocks(HARDFORK_EXEX_102_TIME - fc::hours(1));

  push_op(remove_vault_limit_operation(get_global_properties().authorities.license_administrator,""),true);

  VAULT_ACTOR(vault2);

  BOOST_CHECK(vault1.disable_vault_to_wallet_limit);
  BOOST_CHECK(!vault2.disable_vault_to_wallet_limit);
  generate_blocks(db.head_block_time() + fc::hours(2));

  VAULT_ACTOR(vault3);

  generate_blocks(db.head_block_time() + fc::hours(1));

  BOOST_CHECK(vault1.disable_vault_to_wallet_limit);
  BOOST_CHECK(vault3.disable_vault_to_wallet_limit);
  // this one has to be false because it is created before HARDFORK_EXEX_102_TIME
  // and after removing limit operation
  BOOST_CHECK(!vault2.disable_vault_to_wallet_limit);

  GRAPHENE_REQUIRE_THROW(do_op(update_euro_limit_operation(get_global_properties().authorities.license_administrator,
        vault2_id, true, optional<share_type>(),"comment")), fc::exception);

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

BOOST_AUTO_TEST_CASE(update_global_parameters_test)
{
  try {
  auto root_id = db.get_global_properties().authorities.root_administrator;
  auto lic_id = db.get_global_properties().authorities.license_administrator;
  auto& params = db.get_global_properties().parameters;
  chain_parameters new_params = params;

  // Fails, wrong authority used:
  GRAPHENE_REQUIRE_THROW( do_op(update_global_parameters_operation(lic_id, new_params)), fc::exception );

  // Fails, block interval set to less than 1:
  new_params.block_interval = 0;
  GRAPHENE_REQUIRE_THROW( do_op(update_global_parameters_operation(root_id, new_params)), fc::exception );

  // Fails, maintenance interval less than block interval:
  new_params.block_interval = 6;
  new_params.maintenance_interval = 5;
  GRAPHENE_REQUIRE_THROW( do_op(update_global_parameters_operation(root_id, new_params)), fc::exception );

  // Fails, maintenance interval not a multiply of block interval:
  new_params.maintenance_interval = 11;
  GRAPHENE_REQUIRE_THROW( do_op(update_global_parameters_operation(root_id, new_params)), fc::exception );

  // Success:
  new_params.maintenance_interval = 12;
  do_op(update_global_parameters_operation(root_id, new_params));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // account_unit_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
