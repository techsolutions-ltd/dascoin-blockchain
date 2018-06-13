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
    VAULT_ACTOR(owner)

    // Create a new token
    //do_op(create_)

    const price expected_price{ asset{10, get_cycle_asset_id()}, asset{1 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()} };

    // Create a das33 project
    auto root_id = db.get_global_properties().authorities.root_administrator;
    do_op(das33_project_create_operation(root_id,"test_project1", owner_id, get_dascoin_asset_id(), expected_price, 10000));

    // There should be one inactive das33 project with ratio 10/1
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 1);
    auto& project = get_das33_projects()[0];
    BOOST_CHECK_EQUAL(project.collected.value, 0);
    //BOOST_CHECK_EQUAL(project.cycles_to_token_ratio, std::make_pair(10,1));
    BOOST_CHECK_EQUAL(project.status, das33_project_status::inactive);

//    // Edit status of this project
//    do_op(das33_project_update_operation(root_id, "test_project1", nullptr, nullptr, 10000, 1));
//
//    // Check project
//    project = get_das33_projects()[0];
//    BOOST_CHECK_EQUAL(project.collected.value, 0);
//    //BOOST_CHECK_EQUAL(project.cycles_to_token_ratio, std::make_pair(10,1));
//    BOOST_CHECK_EQUAL(project.status, das33_project_status::active);

    // delete the project
    do_op(das33_project_delete_operation(root_id, project.id));

    // Check that there are no projects
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);
} FC_LOG_AND_RETHROW() }


BOOST_AUTO_TEST_CASE( das33_test )
{ try {

    ACTOR(alice);
    VAULT_ACTOR(alicev);

    tether_accounts(alice_id, alicev_id);

    // Issue a bunch of assets
    issue_dascoin(alicev_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
    issue_dascoin(alice_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
    issue_webasset("1", alice_id, 100, 100);
    BOOST_CHECK_EQUAL( get_balance(alicev_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
    BOOST_CHECK_EQUAL( get_balance(alice_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
    BOOST_CHECK_EQUAL( get_balance(alice_id, get_web_asset_id()), 100 );

    // Check for empty pledges holder object
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::das33_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests

