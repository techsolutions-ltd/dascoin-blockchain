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
#include <graphene/chain/das33_object.hpp>
#include <graphene/chain/market_object.hpp>
#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )
BOOST_FIXTURE_TEST_SUITE( das33_tests, database_fixture )

BOOST_AUTO_TEST_CASE( das33_project_test )
{ try {
    // Check for empty das33 projects
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);

    // Create owner account
    VAULT_ACTOR(owner);

    // Create a new token
    asset_id_type test_asset_id = db.get_index<asset_object>().get_next_id();
    asset_create_operation creator;
    creator.issuer = account_id_type();
    //creator.fee = asset();
    creator.symbol = "TEST";
    creator.common_options.max_supply = 100000000;
    creator.precision = 2;
    creator.common_options.core_exchange_rate = price({asset(1),asset(1,asset_id_type(1))});
    do_op(creator);

    const price expected_price{ asset{10, get_cycle_asset_id()}, asset{100, test_asset_id} };
    vector<price> expected_prices;
    expected_prices.emplace_back( expected_price );

    // Create a das33 project
    auto root_id = db.get_global_properties().authorities.root_administrator;
    do_op(das33_project_create_operation(root_id,"test_project1", owner_id, test_asset_id, expected_prices, 10000));

    // There should be one inactive das33 project with ratio 10/1
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 1);
    auto project = get_das33_projects()[0];
    BOOST_CHECK_EQUAL(project.collected.value, 0);

    BOOST_CHECK_EQUAL(project.token_prices[0].to_real(), 0.1);
    BOOST_CHECK_EQUAL(project.status, das33_project_status::inactive);

    // Edit status of this project
    das33_project_update_operation op;
    op.project_id = project.id;
    op.authority = root_id;
    //op.name = "test_project1";
    op.status = 1;
    do_op(op);
    //do_op(das33_project_update_operation(root_id, "test_project1", owner_id, expected_price, nullptr, 1));

    // Check project
    project = get_das33_projects()[0];
    BOOST_CHECK_EQUAL(project.collected.value, 0);
    BOOST_CHECK_EQUAL(project.token_prices[0].to_real(), 0.1);
    BOOST_CHECK_EQUAL(project.status, das33_project_status::active);

    // delete the project
    do_op(das33_project_delete_operation(root_id, project.id));

    // Check that there are no projects
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);
} FC_LOG_AND_RETHROW() }


BOOST_AUTO_TEST_CASE( das33_cycles_pledge_test )
{ try {

    ACTOR(user);
    VAULT_ACTOR(userv);
    VAULT_ACTOR(owner);

    // Issue standard locked license and some cycles
    auto standard_locked = *(_dal.get_license_type("standard_locked"));
    do_op(issue_license_operation(get_license_issuer_id(), userv_id, standard_locked.id, 50, 20, db.head_block_time()));
    do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), userv_id, standard_locked.id, 100, "foo", "bar"));
    generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

    // Create a das33 project
    vector<price> prices{
        {
            asset{10, get_cycle_asset_id()},
            asset{1 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}
        }
    };
    do_op(das33_project_create_operation(db.get_global_properties().authorities.root_administrator,
                                         "test_project0",
                                         owner_id,
                                         get_dascoin_asset_id(),
                                         prices,
                                         10000));
    das33_project_object project = get_das33_projects()[0];

    // Edit status of this project
    das33_project_update_operation op;
    op.project_id = project.id;
    op.authority = db.get_global_properties().authorities.root_administrator;
    op.status = 1;
    do_op(op);

    // Initial check
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

    // Pledge cycles
    const auto& license_information_obj = (*userv.license_information)(db);
    const auto& license_history = license_information_obj.history;
    const auto& license_record = license_history[0];
    const auto before_pledge = license_record.total_cycles().value;
    do_op(das33_pledge_asset_operation(userv_id, asset{10, get_cycle_asset_id()}, license_record.license, project.id));
    const auto& license_information_obj_2 = (*userv.license_information)(db);
    const auto& license_history_2 = license_information_obj_2.history;
    const auto& license_record_2 = license_history_2[0];
    const auto after_pledge = license_record_2.total_cycles().value;
    BOOST_CHECK_EQUAL(before_pledge - 10, after_pledge);
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 1);

    // Should Fail: when pledging cycles, pledger must be a vault
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(user_id, asset{10, get_cycle_asset_id()}, license_record.license, project.id));, fc::exception );

    // Should Fail: when pledging cycles, license must be provided
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(userv_id, asset{10, get_cycle_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Should Fail: not enough cycles
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(userv_id, asset{5000, get_cycle_asset_id()}, license_record.license, project.id));, fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_dasc_pledge_test )
{ try {

    ACTOR(user);
    VAULT_ACTOR(owner);

    // Issue a bunch of assets
    issue_dascoin(user_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Create a das33 project
    vector<price> prices{
        {
            asset{1 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()},
            asset{1, get_web_asset_id()}
        }
    };
    do_op(das33_project_create_operation(db.get_global_properties().authorities.root_administrator,
                                         "test_project0",
                                         owner_id,
                                         get_web_asset_id(),
                                         prices,
                                         10000));
    das33_project_object project = get_das33_projects()[0];

    // Edit status of this project
    das33_project_update_operation op;
    op.project_id = project.id;
    op.authority = db.get_global_properties().authorities.root_administrator;
    op.status = 1;
    do_op(op);

    // Initial check
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

    // Pledge DASC
    do_op(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    do_op(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);

    // Should Fail: when pledging other than cycles, license must not be provided
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(user_id, asset{50, get_dascoin_asset_id()}, license_type_id_type{}, project.id));, fc::exception );

    // Should Fail: not enough balance
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(user_id, asset{81 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_pledge_test )
{ try {

    ACTOR(user);
    VAULT_ACTOR(owner);

    // Issue asset
    issue_dascoin(user_id, 200000);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 200000 );

    // Should Fail: bad project id
    das33_project_object bad_project;   // default constructed with id 0
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(user_id, asset{100000, get_dascoin_asset_id()}, optional<license_type_id_type>{}, bad_project.id));, fc::exception );

    // Create a das33 project
    vector<price> prices{
            {
                    asset{1, get_dascoin_asset_id()},
                    asset{100000000000, get_web_asset_id()}
            }
    };
    do_op(das33_project_create_operation(db.get_global_properties().authorities.root_administrator,
                                         "test_project0",
                                         owner_id,
                                         get_web_asset_id(),
                                         prices,
                                         10000));
    das33_project_object project = get_das33_projects()[0];

    // Should Fail: exceeding tokens max supply limit
    GRAPHENE_REQUIRE_THROW( do_op(das33_pledge_asset_operation(user_id, asset{100000, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::das33_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests

