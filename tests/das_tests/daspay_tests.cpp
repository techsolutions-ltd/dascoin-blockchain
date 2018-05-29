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
  const auto& root_id = db.get_global_properties().authorities.root_administrator;

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

  const auto& root_id = db.get_global_properties().authorities.root_administrator;
  vector<account_id_type> v{foo_id, bar_id};
  do_op(create_payment_service_provider_operation(root_id, bar_id, v));

  do_op(register_daspay_authority_operation(foo_id, bar_id, pk, {}));

  // This fails - not registered:
  GRAPHENE_REQUIRE_THROW( do_op(unregister_daspay_authority_operation(foo_id, foobar_id)), fc::exception );

  // Success:
  do_op(unregister_daspay_authority_operation(foo_id, bar_id));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::daspay_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests
