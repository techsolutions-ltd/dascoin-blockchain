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
#include <graphene/chain/access_layer.hpp>
#include <graphene/chain/exceptions.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )
BOOST_FIXTURE_TEST_SUITE( cycle_tests, database_fixture )

BOOST_AUTO_TEST_CASE( issue_free_cycles_test )
{ try {
  // VAULT_ACTOR(vault)

  // const auto ISSUER_ID = get_cycle_issuer_id();
  
  // // Issue cycles:
  // do_op(issue_free_cycles_operation(ISSUER_ID, cycle_origin_kind::upgrade, vault_id, 100, "TEST"));

  // // Check the cycle balance:
  // auto vault_balance = db.get_cycle_balance(vault_id);
  // BOOST_CHECK_EQUAL( vault_balance.value, 100 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_cycles_to_license_test )
{ try {
  VAULT_ACTOR(vault);
  VAULT_ACTOR(charter);
  ACTOR(wallet);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto executive_locked = *(_dal.get_license_type("executive_locked"));
  auto lic_typ = *(_dal.get_license_type("standard_charter"));
  const share_type bonus_percent = 50;
  share_type frequency_lock = 20;
  const time_point_sec issue_time = db.head_block_time();
  const uint32_t amount = DASCOIN_BASE_STANDARD_CYCLES + (50 * DASCOIN_BASE_STANDARD_CYCLES) / 100;

    // Issue standard locked license:
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  // This ought to fail, invalid authority:
  GRAPHENE_REQUIRE_THROW( do_op(issue_cycles_to_license_operation(get_license_issuer_id(), vault_id, standard_locked.id, 201, "", "")), fc::exception );

  // This ought to fail, cannot issue cycles to wallet:
  GRAPHENE_REQUIRE_THROW( do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), wallet_id, standard_locked.id, 202, "", "")), fc::exception );

  // This ought to fail, cannot issue cycles to a license which hasn't been issued:
  GRAPHENE_REQUIRE_THROW( do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), vault_id, executive_locked.id, 203, "", "")), fc::exception );

  // This ought to fail, cannot issue zero amount of cycles to a license:
  GRAPHENE_REQUIRE_THROW( do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), vault_id, standard_locked.id, 0, "", "")), fc::exception );

  const auto& license_information_obj = (*vault.license_information)(db);
  const auto& license_history = license_information_obj.history;
  const auto& license_record = license_history[0];

  BOOST_CHECK_EQUAL( license_record.amount.value, amount );
  BOOST_CHECK_EQUAL( license_record.base_amount.value, DASCOIN_BASE_STANDARD_CYCLES );

  // Issue 200 cycles:
  do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), vault_id, standard_locked.id, 200, "foo", "bar"));

  const auto& license_information_obj2 = (*vault.license_information)(db);
  const auto& license_history2 = license_information_obj2.history;
  const auto& license_record2 = license_history2[0];

  // Total cycles should be increased by 200:
  BOOST_CHECK_EQUAL( license_record2.total_cycles().value, amount + 200 );

  // But in fact non_upgradeable_amount is 200:
  BOOST_CHECK_EQUAL( license_record2.non_upgradeable_amount.value, 200 );

  mint_all_dascoin_from_license(lic_typ.id, charter_id);

  BOOST_CHECK_EQUAL( get_balance(charter_id, get_dascoin_asset_id()), 605 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Issue 200 cycles to charter vault:
  do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), charter_id, lic_typ.id, 200, "foo", "bar"));
  toggle_reward_queue(true);

  auto result_vec = *_dal.get_queue_submissions_with_pos(charter_id).result;
  BOOST_CHECK_EQUAL( result_vec.size(), 1 );

  BOOST_CHECK_EQUAL( result_vec[0].position, 0 );

  auto rqo = result_vec[0].submission;
  BOOST_CHECK_EQUAL( rqo.origin, "foo" );
  BOOST_CHECK_EQUAL( rqo.comment, "bar" );

  generate_blocks(db.head_block_time() + fc::hours(24));

  // Now we should have 100 coins more:
  BOOST_CHECK_EQUAL( get_balance(charter_id, get_dascoin_asset_id()), 705 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( purchase_cycle_asset_test )
{ try {
  VAULT_ACTOR(vault);
  ACTOR(wallet);
  ACTOR(feepool);

  tether_accounts(wallet_id, vault_id);

  // Amount of dascoin submitted should be greater than zero:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(vault_id, asset{0, get_dascoin_asset_id()}, 10, 100)), fc::exception );

  // Frequency should be greater than zero:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(vault_id, asset{10, get_dascoin_asset_id()}, 0, 100)), fc::exception );

  // Expected amount should be greater than zero:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(vault_id, asset{10, get_dascoin_asset_id()}, 10, 0)), fc::exception );

  const asset dsc = asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()};

  // Cycles can be bought only from a wallet account:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(vault_id, dsc, 10 * DASCOIN_FREQUENCY_PRECISION, 100)), fc::exception );

  // Default frequency is 2, here 10 is given and that fails:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(wallet_id, dsc, 10 * DASCOIN_FREQUENCY_PRECISION, 100)), fc::exception );

  // No dascoin on the balance, so purchase fails:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(wallet_id, dsc, 2 * DASCOIN_FREQUENCY_PRECISION, 20)), fc::exception );

  issue_dascoin(vault_id, 4000);

  BOOST_CHECK_EQUAL( get_balance(vault_id, get_dascoin_asset_id()), 4000 * DASCOIN_DEFAULT_ASSET_PRECISION );

  db.adjust_balance_limit(vault, get_dascoin_asset_id(), 200 * DASCOIN_DEFAULT_ASSET_PRECISION);
  transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

  BOOST_CHECK_EQUAL( get_balance(wallet_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Calculated price for 20 cycles is 10 dsc, but only 9 sent:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(wallet_id, asset{9 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 2 * DASCOIN_FREQUENCY_PRECISION, 20)), fc::exception );

  // Now 11 dsc is sent:
  GRAPHENE_REQUIRE_THROW( do_op(purchase_cycle_asset_operation(wallet_id, asset{11 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 2 * DASCOIN_FREQUENCY_PRECISION, 20)), fc::exception );

  // Purchase succeeds:
  do_op(purchase_cycle_asset_operation(wallet_id, dsc, 2 * DASCOIN_FREQUENCY_PRECISION, 20));

  // There's 10 dascoin left:
  BOOST_CHECK_EQUAL( get_balance(wallet_id, get_dascoin_asset_id()), 90 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // And we should end up with 20 cycles:
  BOOST_CHECK_EQUAL( get_balance(wallet_id, get_cycle_asset_id()), 20 + DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT );

  // Set fee pool account:
  auto root_id = db.get_global_properties().authorities.root_administrator;
  change_fee_pool_account_operation cfpao;
  cfpao.issuer = root_id;
  cfpao.fee_pool_account_id = feepool_id;
  do_op(cfpao);

  // Purchase succeeds:
  do_op(purchase_cycle_asset_operation(wallet_id, dsc, 2 * DASCOIN_FREQUENCY_PRECISION, 20));

  // Fee pool account should have 10 dsc now:
  BOOST_CHECK_EQUAL( get_balance(feepool_id, get_dascoin_asset_id()), 10 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( transfer_cycles_from_licence_to_wallet_test )
{ try {
  VAULT_ACTOR(vault);
  VAULT_ACTOR(foo);
  ACTOR(wallet);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto executive_locked = *(_dal.get_license_type("executive_locked"));

  // Cannot transfer 0 cycles:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_locked.id, 0, wallet_id)), fc::exception );

  // Cannot transfer from wallet account:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(wallet_id, standard_locked.id, 10, vault_id)), fc::exception );

  // Cannot transfer from wallet account:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_locked.id, 10, foo_id)), fc::exception );

  // Cannot transfer between un-tethered accounts:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_locked.id, 10, wallet_id)), fc::exception );

  tether_accounts(wallet_id, vault_id);

  // Cannot transfer from a vault which doesn't have any license issued:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_locked.id, 10, wallet_id)), fc::exception );

  do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard_locked.id, 10, 2, db.head_block_time()));

  // Cannot transfer from a vault and license which hasn't been issued:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, executive_locked.id, 10, wallet_id)), fc::exception );

  // Cannot transfer more cycles than there is on the vault's license:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_locked.id, 10000, wallet_id)), fc::exception );

  // This will succeed:
  do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_locked.id, 1000, wallet_id));

  // And there should be 1000 cycles on wallet's balance:
  BOOST_CHECK_EQUAL( get_balance(wallet_id, get_cycle_asset_id()), 1000 + DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT );

  const auto& license_information_obj = (*vault.license_information)(db);
  const auto& license_history = license_information_obj.history;
  const auto& license_record = license_history[0];

  // On license there should be 210 cycles remaining:
  BOOST_CHECK_EQUAL( license_record.amount.value, 210 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( transfer_cycles_from_charter_licence_test )
{ try {
  VAULT_ACTOR(vault);
  VAULT_ACTOR(foo);
  ACTOR(wallet);

  auto standard_charter = *(_dal.get_license_type("standard_charter"));
  tether_accounts(wallet_id, vault_id);
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard_charter.id, 10, 2, db.head_block_time()));

  // Cannot transfer cycles from a charter license:
  GRAPHENE_REQUIRE_THROW( do_op(transfer_cycles_from_licence_to_wallet_operation(vault_id, standard_charter.id, 1000, wallet_id)), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::cycle_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
