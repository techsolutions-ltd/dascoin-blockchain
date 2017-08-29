/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/access_layer.hpp>
#include <graphene/chain/exceptions.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )
BOOST_FIXTURE_TEST_SUITE( limit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( get_dascoin_limit_unit_test )
{ try {
  const share_type WEB_AMOUNT = 10 * DASCOIN_FIAT_ASSET_PRECISION;
  const share_type DAS_AMOUNT = 100 * DASCOIN_DEFAULT_ASSET_PRECISION;
  // We use the DAS:WEB market.
  const price DASCOIN_PRICE = 
     asset{DAS_AMOUNT, get_dascoin_asset_id()} / asset{WEB_AMOUNT, get_web_asset_id()};

  optional<share_type> returned_limit;
  optional<share_type> expected_limit;

  // For a wallet account, return nothing:
  ACTOR(wallet);
  returned_limit = db.get_dascoin_limit(wallet_id, DASCOIN_PRICE);
  BOOST_CHECK(!returned_limit.valid());

  // For a vault advocate. return limit based on advocate eur_limit:
  VAULT_ACTOR(advocate);
  const asset ADVOCATE_EUR_LIMIT = asset(DASCOIN_DEFAULT_EUR_LIMIT_ADVOCATE, get_dascoin_asset_id());
  expected_limit = (ADVOCATE_EUR_LIMIT * DASCOIN_PRICE).amount;
  returned_limit = db.get_dascoin_limit(advocate_id, DASCOIN_PRICE);
  BOOST_CHECK(returned_limit.valid());
  BOOST_CHECK_EQUAL(returned_limit->value, expected_limit->value);

  //For a licensed vault account, return limit equal to the appropriate license eur_limit:
  VAULT_ACTOR(president);
  const auto pres_lic = *(_dal.get_license_type("president"));
  const share_type bonus_percent = 50;
  const share_type frequency_lock = 0;
  const time_point_sec issue_time = db.head_block_time();
  do_op(issue_license_operation(get_license_issuer_id(), president_id, pres_lic.id,
        bonus_percent, frequency_lock, issue_time));
  BOOST_CHECK( president.license_information.valid() );

  const asset PRESIDENT_EUR_LIMIT = asset(DASCOIN_DEFAULT_EUR_LIMIT_PRESIDENT, get_dascoin_asset_id());
  expected_limit = (PRESIDENT_EUR_LIMIT * DASCOIN_PRICE).amount;
  returned_limit = db.get_dascoin_limit(president_id, DASCOIN_PRICE);
  BOOST_CHECK(returned_limit.valid());
  BOOST_CHECK_EQUAL(returned_limit->value, expected_limit->value);

} FC_LOG_AND_RETHROW() }


BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::limit_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests