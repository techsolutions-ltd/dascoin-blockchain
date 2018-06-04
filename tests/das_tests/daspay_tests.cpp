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
#include <graphene/chain/daspay_object.hpp>

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

  // Wait for the cycles to be distributed:
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

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  BOOST_CHECK_EQUAL( get_balance(bar_id, get_dascoin_asset_id()), 605 * DASCOIN_DEFAULT_ASSET_PRECISION );

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

  BOOST_CHECK_EQUAL( get_balance(foo_id, get_dascoin_asset_id()), 60 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), 40 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( daspay_credit_test )
{ try {
  ACTORS((foo)(clearing)(payment));
  VAULT_ACTOR(bar);

  tether_accounts(clearing_id, bar_id);

  auto lic_typ = *(_dal.get_license_type("standard_charter"));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, lic_typ.id,
                                10, 200, db.head_block_time()));

  toggle_reward_queue(true);
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));
  db.adjust_balance_limit(bar, get_dascoin_asset_id(), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Generate some coins:
  adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
  adjust_frequency(200);

  // Wait for the cycles to be distributed:
  generate_blocks(db.head_block_time() + fc::seconds(get_chain_parameters().reward_interval_time_seconds));

  // Fails: cannot credit 0 amount
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(bar_id, foo_id, asset{0, db.get_dascoin_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: only web euro can be used to credit:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(bar_id, foo_id, asset{1, db.get_dascoin_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: service provider not found:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(bar_id, foo_id, asset{1, db.get_web_asset_id()}, foo_id, "", {})), fc::exception );

  vector<account_id_type> v{clearing_id};
  const auto& root_id = db.get_global_properties().authorities.daspay_administrator;
  do_op(create_payment_service_provider_operation(root_id, payment_id, v));

  // Fails: clearing account not found:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1, db.get_web_asset_id()}, foo_id, "", {})), fc::exception );

  // Fails: cannot credit vault account:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment_id, bar_id, asset{1, db.get_web_asset_id()}, clearing_id, "", {})), fc::exception );

  // Fails: no funds on clearing account:
  GRAPHENE_REQUIRE_THROW( do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1, db.get_web_asset_id()}, clearing_id, "", {})), fc::exception );

  transfer_dascoin_vault_to_wallet(bar_id, clearing_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

  // Credit one web euro:
  do_op(daspay_credit_account_operation(payment_id, foo_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()}, clearing_id, "", {}));

  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& returned = asset{1 * DASCOIN_FIAT_ASSET_PRECISION, db.get_web_asset_id()} * dgpo.last_dascoin_price;

  BOOST_CHECK_EQUAL( get_reserved_balance(foo_id, get_dascoin_asset_id()), returned.amount.value );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::daspay_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
