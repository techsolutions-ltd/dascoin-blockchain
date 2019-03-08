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
#include <graphene/chain/wire_out_with_fee_object.hpp>
#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( wire_out_with_fee_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( wire_out_with_fee_web_asset_test )
{ try {
  ACTOR(wallet);
  generate_block();
  VAULT_ACTOR(vault);

  tether_accounts(wallet_id, vault_id);
  issue_dascoin(vault_id, 100);
  disable_vault_to_wallet_limit(vault_id);
  transfer_dascoin_vault_to_wallet(vault_id, wallet_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

  const auto check_balances = [this](const account_object& account, share_type expected_cash,
                                     share_type expected_reserved)
  {
    share_type cash, reserved;
    std::tie(cash, reserved) = get_web_asset_amounts(account.id);

    BOOST_CHECK_EQUAL( cash.value, expected_cash.value );
    BOOST_CHECK_EQUAL( reserved.value, expected_reserved.value );
  };

  // Reject, insufficient balance:
  GRAPHENE_REQUIRE_THROW( wire_out_with_fee(wallet_id, web_asset(10000), "BTC", "SOME_BTC_ADDRESS"), fc::exception );

  // Reject, cannot wire out cycles:
  GRAPHENE_REQUIRE_THROW( wire_out_with_fee(wallet_id, asset{1, get_cycle_asset_id()}, "BTC", "SOME_BTC_ADDRESS"), fc::exception );

  // Reject, cannot wire out dascoin:
  GRAPHENE_REQUIRE_THROW( wire_out_with_fee(wallet_id, asset{1, get_dascoin_asset_id()}, "BTC", "SOME_BTC_ADDRESS"), fc::exception );

  issue_webasset("1", wallet_id, 15000, 15000);
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

  // Update the limits:
  // update_pi_limits(wallet_id, 99, {20000,20000,20000});

  // Wire out 10K:
  wire_out_with_fee(wallet_id, web_asset(10000), "BTC", "SOME_BTC_ADDRESS", "debit");

  // Check if the balance has been reduced:
  check_balances(wallet, 5000, 15000);

  // Check if the holder object exists:
  auto holders = get_wire_out_with_fee_holders(wallet_id, {get_web_asset_id()});
  BOOST_CHECK_EQUAL( holders.size(), 1 );
  BOOST_CHECK_EQUAL( holders[0].currency_of_choice, "BTC" );
  BOOST_CHECK_EQUAL( holders[0].to_address, "SOME_BTC_ADDRESS" );
  BOOST_CHECK_EQUAL( holders[0].memo, "debit" );

  // Wire out 5K:
  wire_out_with_fee(wallet_id, web_asset(5000), "BTC", "SOME_BTC_ADDRESS");

  // Check the balances are zero:
  check_balances(wallet, 0, 15000);

  // There should be two holders now:
  holders = get_wire_out_with_fee_holders(wallet_id, {get_web_asset_id()});
  BOOST_CHECK_EQUAL(holders.size(), 2);

  // Deny the first request:
  wire_out_with_fee_reject(holders[0].id);

  // Check if the wire out holder was deleted:
  BOOST_CHECK_EQUAL( get_wire_out_with_fee_holders(wallet_id, {get_web_asset_id()}).size(), 1 );

  // 10K should return to the wallet:
  check_balances(wallet, 10000, 15000);

  // Complete a wire out transaction:
  wire_out_with_fee_complete(holders[1].id);

  // Check if the wire out holder object was deleted:
  BOOST_CHECK_EQUAL( get_wire_out_with_fee_holders(wallet_id, {get_web_asset_id()}).size(), 0 );

  issue_btcasset("1", wallet_id, 15000, 15000);
  wire_out_with_fee(wallet_id, asset{5000, get_btc_asset_id()}, "BTC", "SOME_BTC_ADDRESS");

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( wire_out_with_fee_web_asset_history_test )
{ try {
  ACTOR(wallet);
  generate_block();

  issue_webasset("1", wallet_id, 15000, 15000);
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

  // Wire out 10K:
  wire_out_with_fee(wallet_id, web_asset(10000), "BTC", "SOME_BTC_ADDRESS", "debit");
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));
  auto holders = get_wire_out_with_fee_holders(wallet_id, {get_web_asset_id()});
  wire_out_with_fee_reject(holders[0].id);
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));
  auto history = get_operation_history( wallet_id );
  BOOST_CHECK( !history.empty() );

  // Wire out result should be on top:
  wire_out_with_fee_result_operation op = history[0].op.get<wire_out_with_fee_result_operation>();
  BOOST_CHECK ( !op.completed );

  // Wire out 10K again:
  wire_out_with_fee(wallet_id, web_asset(10000), "BTC", "SOME_BTC_ADDRESS", "debit");
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));
  holders = get_wire_out_with_fee_holders(wallet_id, {get_web_asset_id()});
  wire_out_with_fee_complete(holders[0].id);
  generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));
  history = get_operation_history( wallet_id );

  // Wire out result should be on top:
  op = history[0].op.get<wire_out_with_fee_result_operation>();
  BOOST_CHECK ( op.completed );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( wire_out_with_fee_limit_test )
{ try {
  ACTOR(wallet);

  issue_webasset("1", wallet_id, 2000 * DASCOIN_FIAT_ASSET_PRECISION, 15000);
  auto root_id = db.get_global_properties().authorities.root_administrator;

  // Set withdrawal limit to 500 eur, 100 sec revolving
  auto new_params = db.get_global_properties().parameters;
  new_params.extensions.insert(withdrawal_limit_type{asset{500 * DASCOIN_FIAT_ASSET_PRECISION, asset_id_type{DASCOIN_WEB_ASSET_INDEX}}, 100, {asset_id_type{DASCOIN_WEB_ASSET_INDEX}}});
  do_op(update_global_parameters_operation(root_id, new_params));

  generate_blocks(HARDFORK_BLC_328_TIME + fc::hours(1));

  // Ought to fail, exceeds the absolute limit:
  GRAPHENE_REQUIRE_THROW( wire_out_with_fee(wallet_id, web_asset(600 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit"), fc::exception );

  wire_out_with_fee(wallet_id, web_asset(200 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit");

  // Fails since limit has been exceeded:
  GRAPHENE_REQUIRE_THROW( wire_out_with_fee(wallet_id, web_asset(400 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit"), fc::exception );

  generate_blocks(200);
  wire_out_with_fee(wallet_id, web_asset(400 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit");

  // Fails since time limit has been exceeded:
  GRAPHENE_REQUIRE_THROW( wire_out_with_fee(wallet_id, web_asset(200 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit"), fc::exception );

  generate_blocks(200);
  // Works since the limit has been reset:
  wire_out_with_fee(wallet_id, web_asset(500 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit");

  new_params.extensions.clear();
  new_params.extensions.insert(withdrawal_limit_type{asset{500 * DASCOIN_FIAT_ASSET_PRECISION, asset_id_type{DASCOIN_WEB_ASSET_INDEX}}, 100, {asset_id_type{DASCOIN_DASCOIN_INDEX}}});
  do_op(update_global_parameters_operation(root_id, new_params));

  // Works since eur is no longer limited:
  wire_out_with_fee(wallet_id, web_asset(600 * DASCOIN_FIAT_ASSET_PRECISION), "BTC", "SOME_BTC_ADDRESS", "debit");

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
