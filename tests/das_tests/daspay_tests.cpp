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
#include <graphene/chain/daspay_object.hpp>
#include <graphene/chain/market_object.hpp>
#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )
BOOST_FIXTURE_TEST_SUITE( daspay_tests, database_fixture )

BOOST_AUTO_TEST_CASE( set_daspay_transaction_ratio_test )
{ try {

  // Try to set daspay transaction ratio by using the wrong authority:
  GRAPHENE_REQUIRE_THROW( do_op(set_daspay_transaction_ratio_operation(get_pi_validator_id(), 150, 130)), fc::exception );

  // Try to set daspay transaction ratio with debit and credit ratio illegal (< 10000):
  GRAPHENE_REQUIRE_THROW( do_op(set_daspay_transaction_ratio_operation(get_daspay_administrator_id(), 10000, 10000)), fc::exception );

  do_op(set_daspay_transaction_ratio_operation(get_daspay_administrator_id(), 150, 130));

  auto daspay_debit_transaction_ratio = db.get_dynamic_global_properties().daspay_debit_transaction_ratio;
  auto daspay_credit_transaction_ratio = db.get_dynamic_global_properties().daspay_credit_transaction_ratio;
  BOOST_CHECK_EQUAL( daspay_debit_transaction_ratio.value, 150 );
  BOOST_CHECK_EQUAL( daspay_credit_transaction_ratio.value, 130 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das_payment_service_provider_test )
{ try {

  // Check for empty payment service providers
  BOOST_CHECK_EQUAL(get_payment_service_providers().size(), 0);

  // Create two clearing accounts and one provider account
  ACTORS((provideraccount)(clearingaccount1)(clearingaccount2));
  VAULT_ACTOR(foo);
  vector<account_id_type> v;

  // Require throw (empty clearing_accounts)
  GRAPHENE_REQUIRE_THROW(do_op(create_payment_service_provider_operation(get_daspay_administrator_id(), provideraccount_id, v)), fc::exception );

  v.emplace_back( clearingaccount1_id );
  v.emplace_back( clearingaccount2_id );

  // Require throw (wrong authority)
  GRAPHENE_REQUIRE_THROW(do_op(create_payment_service_provider_operation(get_pi_validator_id(), provideraccount_id, v)), fc::exception );

  // Payment service provider create
  do_op(create_payment_service_provider_operation(get_daspay_administrator_id(), provideraccount_id, v));

  // Check if providers size is 1
  BOOST_CHECK_EQUAL(get_payment_service_providers().size(), 1);
  FC_ASSERT(get_payment_service_providers()[0].payment_service_provider_account == provideraccount_id);
  FC_ASSERT(get_payment_service_providers()[0].payment_service_provider_clearing_accounts == v);

  // Payment service provider delete
  do_op(delete_payment_service_provider_operation(get_daspay_administrator_id(), provideraccount_id));
  BOOST_CHECK_EQUAL(get_payment_service_providers().size(), 0);

  do_op(create_payment_service_provider_operation(get_daspay_administrator_id(), provideraccount_id, v));
  BOOST_CHECK_EQUAL(get_payment_service_providers().size(), 1);

  // Remove one clearing account
  v.erase(std::remove(v.begin(), v.end(), clearingaccount2_id), v.end());
  BOOST_CHECK_EQUAL(v.size(), 1);

  // Fails: payment service provider must be a wallet:
  GRAPHENE_REQUIRE_THROW( do_op(update_payment_service_provider_operation(get_daspay_administrator_id(), foo_id, v)), fc::exception );

  // Payment service provider update (new clearing_accounts)
  do_op(update_payment_service_provider_operation(get_daspay_administrator_id(), provideraccount_id, v));
  FC_ASSERT(get_payment_service_providers()[0].payment_service_provider_clearing_accounts.size() == 1);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( daspay_authority_index_test )
{ try {
  VAULT_ACTORS((foo)(bar));
  ACTORS((wa1)(wa2));

  db.create<daspay_authority_object>([&](daspay_authority_object& dao){
    dao.daspay_user = foo_id;
    dao.payment_provider = wa1_id;
  });

  // This will fail - the same user cannot register the same payment provider twice
  GRAPHENE_REQUIRE_THROW(
    db.create<daspay_authority_object>([&](daspay_authority_object& dao){
      dao.daspay_user = foo_id;
      dao.payment_provider = wa1_id;
    }),
  fc::exception );

  // Success - different payment provider
  db.create<daspay_authority_object>([&](daspay_authority_object& dao){
    dao.daspay_user = foo_id;
    dao.payment_provider = wa2_id;
  });

  // Success - different user
  db.create<daspay_authority_object>([&](daspay_authority_object& dao){
    dao.daspay_user = bar_id;
    dao.payment_provider = wa1_id;
  });

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( delayed_operations_index_test )
{ try {
  ACTORS((wa1)(wa2));

  db.create<delayed_operation_object>([&](delayed_operation_object& dlo){
    dlo.account = wa1_id;
    dlo.issued_time = db.head_block_time();
    dlo.op = unreserve_asset_on_account_operation{wa1_id, asset{ 0, db.get_dascoin_asset_id() } };
  });

  // Fails: cannot create delayed operation of the same type
  GRAPHENE_REQUIRE_THROW(
    db.create<delayed_operation_object>([&](delayed_operation_object& dlo){
      dlo.account = wa1_id;
      dlo.issued_time = db.head_block_time();
      dlo.op = unreserve_asset_on_account_operation{wa1_id, asset{ 0, db.get_dascoin_asset_id() } };
    }),
  fc::exception );

  // Success: different operation
  db.create<delayed_operation_object>([&](delayed_operation_object& dlo){
    dlo.account = wa1_id;
    dlo.issued_time = db.head_block_time();
    dlo.op = reserve_asset_on_account_operation{wa1_id, asset{ 0, db.get_dascoin_asset_id() } };
  });

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( register_daspay_authority_test )
{ try {
  ACTORS((foo)(bar)(foobar)(payment));
  public_key_type pk = public_key_type(generate_private_key("foo").get_public_key());
  const auto& root_id = db.get_global_properties().authorities.daspay_administrator;

  // Cannot register because payment provider is not registered:
  GRAPHENE_REQUIRE_THROW( do_op(register_daspay_authority_operation(foo_id, bar_id, pk, {})), fc::exception );

  vector<account_id_type> v{foo_id, bar_id};
  do_op(create_payment_service_provider_operation(root_id, payment_id, v));
  do_op(create_payment_service_provider_operation(root_id, foobar_id, v));

  // Success - payment provider registered
  do_op(register_daspay_authority_operation(foo_id, payment_id, pk, {}));

  // This fails - payment provider already set:
  GRAPHENE_REQUIRE_THROW( do_op(register_daspay_authority_operation(foo_id, payment_id, pk, {})), fc::exception );

  // This works - payment provider is different:
  do_op(register_daspay_authority_operation(foo_id, foobar_id, pk, {}));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( unregister_daspay_authority_test )
{ try {
  ACTORS((foo)(bar)(foobar));
  public_key_type pk = public_key_type(generate_private_key("foo").get_public_key());

  // This fails - none registered:
  GRAPHENE_REQUIRE_THROW( do_op(unregister_daspay_authority_operation(foo_id, bar_id)), fc::exception );

  const auto& root_id = db.get_global_properties().authorities.daspay_administrator;
  vector<account_id_type> v{foo_id, bar_id};
  do_op(create_payment_service_provider_operation(root_id, bar_id, v));

  do_op(register_daspay_authority_operation(foo_id, bar_id, pk, {}));

  // This fails - not registered:
  GRAPHENE_REQUIRE_THROW( do_op(unregister_daspay_authority_operation(foo_id, foobar_id)), fc::exception );

  // Success:
  do_op(unregister_daspay_authority_operation(foo_id, bar_id));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( reserve_asset_on_account_test )
{ try {
  ACTOR(foo);
  VAULT_ACTOR(bar);

  tether_accounts(foo_id, bar_id);

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Generate some coins:
  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the coins to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  BOOST_CHECK_EQUAL( get_balance(bar_id, get_dascoin_asset_id()), 605 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Fails: only dascoin can be reserved:
  GRAPHENE_REQUIRE_THROW( do_op(reserve_asset_on_account_operation(foo_id, asset{ 10, db.get_web_asset_id() })), fc::exception );

  // Fails: cannot reserve 0 dascoins:
  GRAPHENE_REQUIRE_THROW( do_op(reserve_asset_on_account_operation(foo_id, asset{ 0, db.get_dascoin_asset_id() })), fc::exception );

  // Fails: cannot reserve 0.0001 dascoins, since balance is 0:
  GRAPHENE_REQUIRE_THROW( do_op(reserve_asset_on_account_operation(foo_id, asset{ 10, db.get_dascoin_asset_id() })), fc::exception );

  transfer_dascoin_vault_to_wallet(bar_id, foo_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
  BOOST_CHECK_EQUAL( get_balance(foo_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Fails: only 100 dascoin on balance:
  GRAPHENE_REQUIRE_THROW( do_op(reserve_asset_on_account_operation(foo_id, asset{ 101 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() })), fc::exception );

  // Success:
  do_op(reserve_asset_on_account_operation(foo_id, asset{ 50 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() }));

  BOOST_CHECK_EQUAL( get_balance(foo_id, get_dascoin_asset_id()), 50 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), 50 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( unreserve_asset_on_account_test )
{ try {
  ACTOR(foo);
  VAULT_ACTOR(bar);

  tether_accounts(foo_id, bar_id);

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Generate some coins:
  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the coins to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  BOOST_CHECK_EQUAL( get_balance(bar_id, get_dascoin_asset_id()), 605 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Fails: delayed operations resolver is not running:
  GRAPHENE_REQUIRE_THROW( do_op(unreserve_asset_on_account_operation(foo_id, asset{ 10 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() })), fc::exception );

  do_op(update_delayed_operations_resolver_parameters_operation(db.get_global_properties().authorities.root_administrator, true, 600));

  // Fails: only dascoin can be unreserved:
  GRAPHENE_REQUIRE_THROW( do_op(unreserve_asset_on_account_operation(foo_id, asset{ 10, db.get_web_asset_id() })), fc::exception );

  // Fails: cannot unreserve 0 dascoins:
  GRAPHENE_REQUIRE_THROW( do_op(unreserve_asset_on_account_operation(foo_id, asset{ 0, db.get_dascoin_asset_id() })), fc::exception );

  // Fails: cannot unreserve 0.0001 dascoins, since reserved balance is 0:
  GRAPHENE_REQUIRE_THROW( do_op(unreserve_asset_on_account_operation(foo_id, asset{ 10, db.get_dascoin_asset_id() })), fc::exception );

  transfer_dascoin_vault_to_wallet(bar_id, foo_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Reserve 50:
  do_op(reserve_asset_on_account_operation(foo_id, asset{ 50 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() }));

  // Fails: cannot unreserve 60 dascoins, since reserved balance is 50:
  GRAPHENE_REQUIRE_THROW( do_op(unreserve_asset_on_account_operation(foo_id, asset{ 60 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() })), fc::exception );

  do_op(unreserve_asset_on_account_operation(foo_id, asset{ 10 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() }));

  // Fails: cannot issue another unreserve operation before the first one is resolved:
  GRAPHENE_REQUIRE_THROW( do_op(unreserve_asset_on_account_operation(foo_id, asset{ 10 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() })), fc::exception );

  // Unreserve resolver is not working, so no change on the balance:
  BOOST_CHECK_EQUAL( get_balance(foo_id, get_dascoin_asset_id()), 50 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), 50 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Wait for delayed operations resolver to kick in:
  generate_blocks(db.head_block_time() + fc::seconds(660));

  auto history = get_operation_history( foo_id );
  BOOST_CHECK( !history.empty() );
  // unreserve_completed should be on top:
  unreserve_completed_operation op = history[0].op.get<unreserve_completed_operation>();
  BOOST_CHECK_EQUAL ( op.asset_to_unreserve.amount.value,  asset( 10 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() ).amount.value );

  BOOST_CHECK_EQUAL( get_balance(foo_id, get_dascoin_asset_id()), 60 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), 40 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( daspay_debit_test )
{ try {
  ACTORS((foo)(clearing1)(payment1)(clearing2)(payment2)(foobar));
  VAULT_ACTOR(bar);

  tether_accounts(foo_id, bar_id);

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Generate some coins:
  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the coins to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  public_key_type pk1 = public_key_type(generate_private_key("foo").get_public_key());
  vector<account_id_type> v1{clearing1_id};

  // Fails: only web euro can be used to debit:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{0, db.get_dascoin_asset_id()}, clearing1_id, "", {})), fc::exception );

  // Fails: service provider not found:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(bar_id, pk1, foo_id, asset{0, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

  do_op(create_payment_service_provider_operation(get_daspay_administrator_id(), payment1_id, v1));

  // Fails: user has not enabled daspay:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{1, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

  do_op(register_daspay_authority_operation(foo_id, payment1_id, pk1, {}));

  // Fails: clearing account not found:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{1, db.get_web_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: cannot debit vault account:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk1, bar_id, asset{1, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

  // Fails: no funds on user account:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{1, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

  transfer_dascoin_vault_to_wallet(bar_id, foo_id, 600 * DASCOIN_DEFAULT_ASSET_PRECISION);
  do_op(reserve_asset_on_account_operation(foo_id, asset{ 600 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() }));

  public_key_type pk2 = public_key_type(generate_private_key("foo2").get_public_key());

  // Fails: wrong key used:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk2, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

  // Set debit transaction ratio to 2.0%
  do_op(set_daspay_transaction_ratio_operation(get_daspay_administrator_id(), 200, 0));

  // Set price to 1we -> 100dasc
  set_last_dascoin_price(asset(100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

  // Fails: cannot debit negative amount:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{-1, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

  // Success, we can debit 0 amount:
  do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{0, db.get_web_asset_id()}, clearing1_id, "", {}));

  issue_webasset("1", foobar_id, 1 * DASCOIN_FIAT_ASSET_PRECISION, 0);
  do_op(limit_order_create_operation(foobar_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 0, {}, db.head_block_time() + fc::seconds(600)));

  // Debit one web euro:
  do_op(daspay_debit_account_operation(payment1_id, pk1, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing1_id, "", {}));

  const auto& dgpo = db.get_dynamic_global_properties();
  share_type debit_amount_with_fee = 1 * DASCOIN_FIAT_ASSET_PRECISION;
  debit_amount_with_fee += debit_amount_with_fee * db.get_dynamic_global_properties().daspay_debit_transaction_ratio / 10000;
  const auto& debit_amount = asset{debit_amount_with_fee, db.get_web_asset_id()} * dgpo.last_dascoin_price;

  BOOST_CHECK_EQUAL( get_dascoin_balance(clearing1_id), debit_amount.amount.value );

  vector<account_id_type> v2{clearing2_id};
  do_op(create_payment_service_provider_operation(get_daspay_administrator_id(), payment2_id, v2));
  do_op(register_daspay_authority_operation(foo_id, payment2_id, pk2, {}));
  do_op(daspay_debit_account_operation(payment2_id, pk2, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing2_id, "", {}));
  BOOST_CHECK_EQUAL( get_dascoin_balance(clearing2_id), debit_amount.amount.value );

  // Fails: wrong key used (pk2 is registered with payment2_id):
  GRAPHENE_REQUIRE_THROW( do_op(daspay_debit_account_operation(payment1_id, pk2, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing1_id, "", {})), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( daspay_credit_test )
{ try {
  ACTORS((foo)(clearing)(payment)(payment2)(foobar));
  VAULT_ACTORS((bar)(foobar2));

  tether_accounts(clearing_id, bar_id);
  tether_accounts(foobar_id, foobar2_id);

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Generate some coins:
  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the coins to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Fails: cannot credit 0 amount
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(bar_id, foo_id, asset{0, db.get_dascoin_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: only web euro can be used to credit:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(bar_id, foo_id, asset{1, db.get_dascoin_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: service provider not found:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(bar_id, foo_id, asset{1, db.get_web_asset_id()}, foo_id, "", {})), fc::exception );

  vector<account_id_type> v{clearing_id};
  const auto& root_id = db.get_global_properties().authorities.daspay_administrator;
  public_key_type pk = public_key_type(generate_private_key("foo").get_public_key());

  do_op(create_payment_service_provider_operation(root_id, payment_id, v));
  do_op(create_payment_service_provider_operation(root_id, payment2_id, v));
  do_op(register_daspay_authority_operation(foo_id, payment_id, pk, {}));

  // Fails: clearing account not found:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1, db.get_web_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: cannot credit vault account:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment_id, bar_id, asset{1, db.get_web_asset_id()}, clearing_id, "", {})), fc::exception );

  // Fails: no funds on clearing account:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1, db.get_web_asset_id()}, clearing_id, "", {})), fc::exception );

  transfer_dascoin_vault_to_wallet(bar_id, clearing_id, 200 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Set credit transaction ratio to 2.0%
  do_op(set_daspay_transaction_ratio_operation(get_daspay_administrator_id(), 0, 200));

  // Set price to 1we -> 100dasc
  set_last_dascoin_price(asset(100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

  issue_dascoin(foobar2_id, 1000);
  disable_vault_to_wallet_limit(foobar2_id);
  transfer_dascoin_vault_to_wallet(foobar2_id, foobar_id, 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  do_op(limit_order_create_operation(foobar_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()}, 0, {}, db.head_block_time() + fc::seconds(600)));

  // Credit one web euro:
  do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing_id, "", {}));

  const auto& dgpo = db.get_dynamic_global_properties();
  share_type credit_amount_with_fee = 1 * DASCOIN_FIAT_ASSET_PRECISION;
  credit_amount_with_fee += credit_amount_with_fee * db.get_dynamic_global_properties().daspay_credit_transaction_ratio / 10000;
  const auto& credit_amount = asset{credit_amount_with_fee, db.get_web_asset_id()} * dgpo.last_dascoin_price;

  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), credit_amount.amount.value );

  // Fails: use has not enabled payment2 as a payment provider:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment2_id, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing_id, "", {})), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( update_daspay_clearing_parameters_unit_test )
{ try {

  do_op(update_daspay_clearing_parameters_operation(get_daspay_administrator_id(),
                                                    {true},
                                                    {600},
                                                    {90000 * DASCOIN_FIAT_ASSET_PRECISION},
                                                    {250000 * DASCOIN_DEFAULT_ASSET_PRECISION}));

  const auto& daspay_params = get_daspay_parameters();
  BOOST_CHECK_EQUAL( daspay_params.clearing_enabled, true );
  BOOST_CHECK_EQUAL( daspay_params.clearing_interval_time_seconds, 600 );
  BOOST_CHECK_EQUAL( daspay_params.collateral_dascoin.value, 90000 * DASCOIN_FIAT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( daspay_params.collateral_webeur.value, 250000 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( daspay_clearing_test )
{ try {
  ACTORS((foo)(clearing)(payment)(buyer));
  VAULT_ACTORS((bar)(foobar));

  tether_accounts(foo_id, bar_id);
  tether_accounts(clearing_id, foobar_id);

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  do_op(issue_license_operation(get_license_issuer_id(), foobar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Generate some coins:
  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the coins to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  vector<account_id_type> v{clearing_id};
  const auto& root_id = db.get_global_properties().authorities.daspay_administrator;
  public_key_type pk = public_key_type(generate_private_key("foo").get_public_key());

  do_op(create_payment_service_provider_operation(root_id, payment_id, v));
  do_op(register_daspay_authority_operation(foo_id, payment_id, pk, {}));

  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  db.adjust_balance_limit(foobar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  transfer_dascoin_vault_to_wallet(bar_id, foo_id, 200 * DASCOIN_DEFAULT_ASSET_PRECISION);
  transfer_dascoin_vault_to_wallet(foobar_id, clearing_id, 200 * DASCOIN_DEFAULT_ASSET_PRECISION);
  do_op(reserve_asset_on_account_operation(foo_id, asset{ 200 * DASCOIN_DEFAULT_ASSET_PRECISION, db.get_dascoin_asset_id() }));

  // Set debit transaction ratio to 2.0%
  do_op(set_daspay_transaction_ratio_operation(get_daspay_administrator_id(), 200, 0));

  // Set price to 1we -> 100dasc
  set_last_dascoin_price(asset(100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));
  issue_webasset("1", buyer_id, 1 * DASCOIN_FIAT_ASSET_PRECISION, 0);
  do_op(limit_order_create_operation(buyer_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 0, {}, db.head_block_time() + fc::seconds(600)));

  // Debit one web euro:
  do_op(daspay_debit_account_operation(payment_id, pk, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing_id, "", {}));

  // Since price is 100 dasc for 1 web eur and transaction ratio is 2.0%, we took 102 dasc from foo's reserved balance:
  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), 98 * DASCOIN_DEFAULT_ASSET_PRECISION );

  const auto& limit_order_idx = db.get_index_type<limit_order_index>();
  const auto& limit_price_idx = limit_order_idx.indices().get<by_price>();

  // Onelimit order at this point:
  BOOST_CHECK_EQUAL( limit_price_idx.size(), 1 );

  // Enable daspay clearing:
  do_op(update_daspay_clearing_parameters_operation(get_daspay_administrator_id(), true, 12, {}, {}));

  // Wait for the next clearing interval:
  generate_blocks(db.head_block_time() + fc::seconds(18));

  // No limit orders because a match has been made:
  BOOST_CHECK_EQUAL( limit_price_idx.size(), 0 );

  issue_webasset("2", foo_id, 100 * DASCOIN_FIAT_ASSET_PRECISION, 0);
  do_op(limit_order_create_operation(foo_id, asset{10 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 0, {}, db.head_block_time() + fc::seconds(600)));

  // Wait for the next clearing interval:
  generate_blocks(db.head_block_time() + fc::seconds(18));

  auto history = get_operation_history(foo_id);
  BOOST_CHECK( !history.empty() );
  fill_order_operation fo = history[0].op.get<fill_order_operation>();
  BOOST_CHECK( fo.account_id == foo_id );
  BOOST_CHECK( fo.fill_price == price(asset(10 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()), asset(100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id())));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( daspay_clearing2_test )
{ try {
  ACTORS((foo)(clearing)(payment));
  VAULT_ACTORS((bar)(foobar));

  tether_accounts(foo_id, bar_id);
  tether_accounts(clearing_id, foobar_id);

  vector<account_id_type> v{clearing_id};
  const auto& root_id = db.get_global_properties().authorities.daspay_administrator;
  public_key_type pk = public_key_type(generate_private_key("foo").get_public_key());

  do_op(create_payment_service_provider_operation(root_id, payment_id, v));
  do_op(register_daspay_authority_operation(foo_id, payment_id, pk, {}));

  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  db.adjust_balance_limit(foobar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  issue_dascoin(bar_id, 1000);
  issue_dascoin(foobar_id, 500);

  transfer_dascoin_vault_to_wallet(bar_id, foo_id, 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);
  transfer_dascoin_vault_to_wallet(foobar_id, clearing_id, 500 * DASCOIN_DEFAULT_ASSET_PRECISION);

  issue_webasset("1", clearing_id, 100 * DASCOIN_FIAT_ASSET_PRECISION, 0);

  // Set huge dasc collateral so no sells will be made:
  do_op(update_daspay_clearing_parameters_operation(get_daspay_administrator_id(), {}, {}, 1000 * DASCOIN_DEFAULT_ASSET_PRECISION, {}));

  do_op(limit_order_create_operation(foo_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, asset{7 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()}, 0, {}, db.head_block_time() + fc::seconds(600)));

  BOOST_CHECK_EQUAL( get_balance(clearing_id, get_dascoin_asset_id()), 500 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Set price to 1we -> 100dasc
  set_last_dascoin_price(asset(100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

  // Return 1 web euro back:
  do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing_id, "", {}));

  // 100 dascoins were returned, 400 left:
  BOOST_CHECK_EQUAL( get_balance(clearing_id, get_dascoin_asset_id()), 400 * DASCOIN_DEFAULT_ASSET_PRECISION );

  // Set huge dasc collateral so no sells will be made:
  do_op(update_daspay_clearing_parameters_operation(get_daspay_administrator_id(), true, {}, {}, {}));

  // Wait for the next clearing interval:
  generate_blocks(db.head_block_time() + fc::seconds(12));

  const auto& limit_order_idx = db.get_index_type<limit_order_index>();
  const auto& limit_price_idx = limit_order_idx.indices().get<by_price>();

  // At this point there should be one limit order:
  BOOST_CHECK_EQUAL( limit_price_idx.size(), 1 );
  const auto& loo = *(limit_price_idx.begin());

  // Seller is clearing account:
  BOOST_CHECK( loo.seller == clearing_id );

  // Price is 7 cents, so we are selling 42 euros for 600 dascoins (remember, collateral is 1000 dascoins):
  BOOST_CHECK_EQUAL( loo.sell_price.base.amount.value, 42 * DASCOIN_FIAT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( loo.sell_price.quote.amount.value, 600 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( update_delayed_operations_resolver_parameters_unit_test )
{ try {

  do_op(update_delayed_operations_resolver_parameters_operation(db.get_global_properties().authorities.root_administrator,
                                                    {true},
                                                    {600}));

  BOOST_CHECK_EQUAL( db.get_global_properties().delayed_operations_resolver_enabled, true );
  BOOST_CHECK_EQUAL( db.get_global_properties().delayed_operations_resolver_interval_time_seconds, 600 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::daspay_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
