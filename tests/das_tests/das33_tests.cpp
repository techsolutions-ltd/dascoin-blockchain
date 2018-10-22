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
    ACTOR(owner);

    // Create a new token
    asset_id_type test_asset_id = db.get_index<asset_object>().get_next_id();
    asset_create_operation creator;
    creator.issuer = db.get_global_properties().authorities.webasset_issuer;
    //creator.fee = asset();
    creator.symbol = "TEST";
    creator.common_options.max_supply = 100000000;
    creator.precision = 2;
    creator.common_options.core_exchange_rate = price({asset(1),asset(1,asset_id_type(1))});
    do_op(creator);

    // Create a das33 project
    auto das33_admin_id = get_das33_administrator_id();
    do_op(das33_project_create_operation(das33_admin_id,"test_project1", owner_id, test_asset_id, 10000000, {{get_dascoin_asset_id(),50}}));

    // There should be one inactive das33 project with ratio 1/10
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 1);
    auto project = get_das33_projects()[0];
    BOOST_CHECK_EQUAL(project.collected_amount_eur.value, 0);

    BOOST_CHECK_EQUAL(project.token_price.to_real(), 0.1);
    BOOST_CHECK_EQUAL(project.status, das33_project_status::inactive);

    // Edit status of this project
    das33_project_update_operation op;
    op.project_id = project.id;
    op.authority = das33_admin_id;
    op.status = 1;
    do_op(op);

    // Check project
    project = get_das33_projects()[0];
    BOOST_CHECK_EQUAL(project.collected_amount_eur.value, 0);
    BOOST_CHECK_EQUAL(project.token_price.to_real(), 0.1);
    BOOST_CHECK_EQUAL(project.status, das33_project_status::active);

    // delete the project
    do_op(das33_project_delete_operation(das33_admin_id, project.id));

    // Check that there are no projects
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_project_test_fails )
{ try {
    // Check for empty das33 projects
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);

    // Create owner account
    ACTOR(owner);

    // Set authorities
    auto das33_admin_id = get_das33_administrator_id();
    auto issuer_id = db.get_global_properties().authorities.webasset_issuer;

    asset_id_type test_asset_id = create_new_asset("TEST", 100000000, 2, price({asset(1),asset(1,asset_id_type(1))}));

    // Create discounts map
    map<asset_id_type, share_type> discounts{
      {
        get_dascoin_asset_id(),
        100
      }
    };

    // Project with wrong issuer
    GRAPHENE_REQUIRE_THROW(
        do_op(das33_project_create_operation(issuer_id, "test_project1", owner_id, test_asset_id, 10000, discounts)),
        fc::exception );
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);

    // Project with wrong token id
    GRAPHENE_REQUIRE_THROW(
        do_op(das33_project_create_operation(das33_admin_id, "test_project1", owner_id, db.get_index<asset_object>().get_next_id(), 10000, discounts)),
        fc::exception );
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);

    // Project with DASC as token
    GRAPHENE_REQUIRE_THROW(
        do_op(das33_project_create_operation(das33_admin_id, "test_project1", owner_id, get_dascoin_asset_id(), 10000, discounts)),
        fc::exception );
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);

    // Project with wrong goal amount
    GRAPHENE_REQUIRE_THROW(
        do_op(das33_project_create_operation(das33_admin_id, "test_project1", owner_id, test_asset_id, 0, discounts)),
        fc::exception );
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);

    // Create a das33 project
    do_op(das33_project_create_operation(das33_admin_id,"test_project1", owner_id, test_asset_id, 10000, discounts));
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 1);

    // Project with same name
    GRAPHENE_REQUIRE_THROW(
        do_op(das33_project_create_operation(das33_admin_id, "test_project1", owner_id, test_asset_id, 10000, discounts)),
        fc::exception );
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 1);

    // Project with same token
    GRAPHENE_REQUIRE_THROW(
        do_op(das33_project_create_operation(das33_admin_id, "test_project2", owner_id, test_asset_id, 10000, discounts)),
        fc::exception );
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 1);
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_pledge_dasc_test )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue a bunch of assets
    issue_dascoin(vault_id, 100);
    disable_vault_to_wallet_limit(vault_id);
    transfer_dascoin_vault_to_wallet(vault_id, user_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Create project token
    //   max_supply is 1.000.000,00000
    //   precision is 5
    asset_id_type test_asset_id = create_new_asset("TEST", 100000000000, 5, price({asset(1),asset(1,asset_id_type(1))}));

    // Create a das33 project
    das33_project_create_operation project_create;
        project_create.authority       = get_das33_administrator_id();
        project_create.name            = "test_project0";
        project_create.owner           = owner_id;
        project_create.token           = test_asset_id;
        project_create.discounts       = {{get_dascoin_asset_id(), 60}};
        project_create.goal_amount_eur = 10000000; // 100.000 with precision 2
        project_create.min_pledge      = 0;
        project_create.max_pledge      = 10000000000; // 100.000 with precision 5
    do_op(project_create);

    das33_project_object project = get_das33_projects()[0];

    // Activate project
    das33_project_update_operation project_update;
        project_update.project_id = project.id;
        project_update.authority  = get_das33_administrator_id();
        project_update.status     = das33_project_status::active;
    do_op(project_update);

    // Initial check
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

    // Set last dascoin price
    set_last_dascoin_price(asset(1 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

    // Pledge DASC
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);

    // Check pledges
    vector<das33_pledge_holder_object> pledges = get_das33_pledges();
    BOOST_CHECK_EQUAL(pledges.size(), 2);

    BOOST_CHECK(pledges[0].pledged.amount          == 1000000); // 10 with precision 5
    BOOST_CHECK(pledges[0].pledge_remaining.amount == 1000000); // 10 with precision 5
    BOOST_CHECK(pledges[0].base_expected.amount    == 10000000); // 100 with precision 5
    BOOST_CHECK(pledges[0].base_remaining.amount   == 10000000); // 100 with precision 5
    BOOST_CHECK(pledges[0].bonus_expected.amount   == 6666666); // 66.66666 with precision 5
    BOOST_CHECK(pledges[0].bonus_remaining.amount  == 6666666); // 66.66666 with precision 5

    BOOST_CHECK(pledges[1].pledged.amount          == 1000000); // 10 with precision 5
    BOOST_CHECK(pledges[1].pledge_remaining.amount == 1000000); // 10 with precision 5
    BOOST_CHECK(pledges[1].base_expected.amount    == 10000000); // 100 with precision 5
    BOOST_CHECK(pledges[1].base_remaining.amount   == 10000000); // 100 with precision 5
    BOOST_CHECK(pledges[1].bonus_expected.amount   == 6666666); // 66.6666 with precision 5
    BOOST_CHECK(pledges[1].bonus_remaining.amount  == 6666666); // 66.6666 with precision 5

    // Should Fail: not enough balance
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{81 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Complete project
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 0, 10000, 10000, 10000));
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_pledge_dasc_test_min_max )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue a bunch of assets
    issue_dascoin(vault_id, 1000);
    disable_vault_to_wallet_limit(vault_id);
    transfer_dascoin_vault_to_wallet(vault_id, user_id, 1000 * DASCOIN_DEFAULT_ASSET_PRECISION);

    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 1000 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Create a das33 project
    asset_id_type test_asset_id = create_new_asset("TEST", 10000000, 2, price({asset(1),asset(1,asset_id_type(1))}));

    das33_project_create_operation project_create;
        project_create.authority       = get_das33_administrator_id();
        project_create.name            = "test_project0";
        project_create.owner           = owner_id;
        project_create.token           = test_asset_id;
        project_create.discounts       = {{get_dascoin_asset_id(), 50}};
        project_create.goal_amount_eur = 10000000;
        project_create.min_pledge      = 5000;
        project_create.max_pledge      = 20000;
    do_op(project_create);

    das33_project_object project = get_das33_projects()[0];

    // Activate project
    das33_project_update_operation project_update;
        project_update.project_id = project.id;
        project_update.authority  = get_das33_administrator_id();
        project_update.status     = das33_project_status::active;
    do_op(project_update);

    // Initial check
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

    // Set last dascoin price
    set_last_dascoin_price(asset(1 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

    // Should Fail: amount less than min_pledge
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{49.99999 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Pledge DASC
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);

    // Should Fail: more than max_pledge in current round
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{0.00001 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Complete project
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 0, 10000, 10000, 10000));
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_pledge_test_phase_limit )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue asset
    issue_dascoin(vault_id, 2000);
    disable_vault_to_wallet_limit(vault_id);
    transfer_dascoin_vault_to_wallet(vault_id, user_id, 2000 * DASCOIN_DEFAULT_ASSET_PRECISION);

    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 200000000 );

    // Should Fail: bad project id
    das33_project_object bad_project;   // default constructed with id 0
    BOOST_CHECK_EQUAL(get_das33_projects().size(), 0);
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100000, get_dascoin_asset_id()}, optional<license_type_id_type>{}, bad_project.id));, fc::exception );

    // Create a das33 project
    asset_id_type test_asset_id = create_new_asset("TEST", 100000, 2, price({asset(1),asset(1,asset_id_type(1))}));

    // Set last dascoin price
    set_last_dascoin_price(asset(2 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

    price last_dsc_price = get_dynamic_global_properties().last_dascoin_price;
    BOOST_CHECK_EQUAL( last_dsc_price.to_real(), 2000);

    // Create project
    das33_project_create_operation project_create;
        project_create.authority       = get_das33_administrator_id();
        project_create.name            = "test_project0";
        project_create.owner           = owner_id;
        project_create.token           = test_asset_id;
        project_create.discounts        = {{get_dascoin_asset_id(), 50}};
        project_create.goal_amount_eur = 100000;
        project_create.min_pledge      = 0;
        project_create.max_pledge      = 1000000;
    do_op(project_create);

    das33_project_object project = get_das33_projects()[0];

    // Should Fail: project inactive
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{1, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Activate project & set phase limit
    das33_project_update_operation project_update;
        project_update.project_id = project.id;
        project_update.authority  = get_das33_administrator_id();
        project_update.phase_limit = 20000;
        project_update.status     = das33_project_status::active;
    do_op(project_update);

    // Should Work: pledge 100 DSC (50 WE)
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 1);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 1900 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Should Work: attempt to pledge 200 DSC (100 WE), but actually pledge 100 DSC (50 WE)
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{200 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 1800 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Should Fail: all tokens for this phase are sold
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100000, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Complete project
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 0, 10000, 10000, 10000));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_pledge_test_overflow )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue asset
    issue_dascoin(vault_id, 200000);
    disable_vault_to_wallet_limit(vault_id);
    transfer_dascoin_vault_to_wallet(vault_id, user_id, 200000 * DASCOIN_DEFAULT_ASSET_PRECISION);

    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 20000000000 );

    // Create a das33 project
    asset_id_type test_asset_id = create_new_asset("TEST", 40000000000000, 5, price({asset(1),asset(1,asset_id_type(1))}));

    // Set last dascoin price
    set_last_dascoin_price(asset(999999 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()) / asset(9999990000 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));

    // Create project
    das33_project_create_operation project_create;
        project_create.authority       = get_das33_administrator_id();
        project_create.name            = "test_project0";
        project_create.owner           = owner_id;
        project_create.token           = test_asset_id;
        project_create.discounts        = {{get_dascoin_asset_id(), 50}};
        project_create.goal_amount_eur = 40000000000;
        project_create.min_pledge      = 0;
        project_create.max_pledge      = 40000000000000;
    do_op(project_create);

    das33_project_object project = get_das33_projects()[0];

    // Should Fail: project inactive
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{1, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Activate project & set phase limit
    das33_project_update_operation project_update;
        project_update.project_id = project.id;
        project_update.authority  = get_das33_administrator_id();
        project_update.status     = das33_project_status::active;
    do_op(project_update);

    // Should Work: pledge 10.000 DSC (100.000.000 WE)
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10000 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 1);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 190000 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Should Work: attempt to pledge 20.000 DSC (200.000.000 WE), but actually pledge 10.000 DSC (100.000.000 WE)
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{20000 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 180000 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Should Fail: all tokens are sold
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100000, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    // Complete project
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 0, 10000, 10000, 10000));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_pledge_test_bitcoin )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue asset
    issue_asset("test0", user_id, 200 * DASCOIN_BITCOIN_PRECISION, 0, get_btc_asset_id(), get_webasset_issuer_id(), "test");

    BOOST_CHECK_EQUAL( get_balance(user_id, get_btc_asset_id()), 200 * DASCOIN_BITCOIN_PRECISION );

    // Create a das33 project
    asset_id_type test_asset_id = create_new_asset("TEST", 200000000000, 5, price({asset(1),asset(1,asset_id_type(1))}));

    // Set last dascoin price
   // set_external_bitcoin_price(asset(1 * DASCOIN_BITCOIN_PRECISION, get_btc_asset_id()) / asset(10000 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()));
    update_external_token_price_operation uepo;
    uepo.issuer = get_webasset_issuer_id();
    uepo.token_id = get_btc_asset_id();
    uepo.eur_amount_per_token = asset(1 * DASCOIN_BITCOIN_PRECISION, get_btc_asset_id()) / asset(10000 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id());
    do_op(uepo);

    // Create project
    das33_project_create_operation project_create;
        project_create.authority       = get_das33_administrator_id();
        project_create.name            = "test_project0";
        project_create.owner           = owner_id;
        project_create.token           = test_asset_id;
        project_create.discounts        = {{get_dascoin_asset_id(), 50}};
        project_create.goal_amount_eur = 200000000;
        project_create.min_pledge      = 0;
        project_create.max_pledge      = 200000000000;
    do_op(project_create);

    das33_project_object project = get_das33_projects()[0];

    // Activate project
    das33_project_update_operation project_update;
        project_update.project_id = project.id;
        project_update.authority  = get_das33_administrator_id();
        project_update.status     = das33_project_status::active;
    do_op(project_update);

    // Should Fail: no bonus set for BTC
    GRAPHENE_REQUIRE_THROW( do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{1 * DASCOIN_BITCOIN_PRECISION, get_btc_asset_id()}, optional<license_type_id_type>{}, project.id));, fc::exception );

    map<asset_id_type, share_type> bonus_map = {{get_btc_asset_id(), 100}};
    // Add BTC bonus
    das33_project_update_operation project_update2;
        project_update2.project_id   = project.id;
        project_update2.authority    = get_das33_administrator_id();
        project_update2.phase_number = 1;
        project_update2.discounts    = bonus_map;
    do_op(project_update2);

    // Should Work: pledge 10 BTC (100.000 WE)
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_BITCOIN_PRECISION, get_btc_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 1);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_btc_asset_id()), 190 * DASCOIN_BITCOIN_PRECISION );

    // Check pledges
    vector<das33_pledge_holder_object> pledges = get_das33_pledges();
        BOOST_CHECK_EQUAL(pledges.size(), 1);

    BOOST_CHECK(pledges[0].pledged.amount          == 10 * DASCOIN_BITCOIN_PRECISION); // 10 with precision 8
    BOOST_CHECK(pledges[0].pledged.asset_id        == get_btc_asset_id()); // BTC
    BOOST_CHECK(pledges[0].pledge_remaining.amount == 10 * DASCOIN_BITCOIN_PRECISION); // 10 with precision 8
    BOOST_CHECK(pledges[0].base_expected.amount    == 10000000000); // 100000 with precision 5
    BOOST_CHECK(pledges[0].base_remaining.amount   == 10000000000); // 100000 with precision 5
    BOOST_CHECK(pledges[0].bonus_expected.amount   == 0); // 0
    BOOST_CHECK(pledges[0].bonus_remaining.amount  == 0); // 0

    // Should Work: attempt to pledge 100 BTC (1.000.000 WE)
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{100 * DASCOIN_BITCOIN_PRECISION, get_btc_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_btc_asset_id()), 90 * DASCOIN_BITCOIN_PRECISION );

    // Complete project
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 1, 10000, 10000, 10000));

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_complete_project_test )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue a bunch of assets
    issue_dascoin(vault_id, 100);
    disable_vault_to_wallet_limit(vault_id);
    transfer_dascoin_vault_to_wallet(vault_id, user_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Create a das33 project
    asset_id_type test_asset_id = create_new_asset("TEST", 100000000, 2, price({asset(1),asset(1,asset_id_type(1))}));

    das33_project_create_operation project_create;
        project_create.authority       = get_das33_administrator_id();
        project_create.name            = "test_project0";
        project_create.owner           = owner_id;
        project_create.token           = test_asset_id;
        project_create.discounts       = {{get_dascoin_asset_id(), 50}};
        project_create.goal_amount_eur = 10000000;
        project_create.min_pledge = 0;
        project_create.max_pledge = 10000000;
    do_op(project_create);

    das33_project_object project = get_das33_projects()[0];

    // Activate project
    das33_project_update_operation project_update;
        project_update.project_id = project.id;
        project_update.authority  = get_das33_administrator_id();
        project_update.status     = das33_project_status::active;
    do_op(project_update);

    // Initial check
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

    // Pledge DASC in phase 0
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 3);

    // Complete first pledge: <100%, 100%,  5% >
    do_op_no_balance_check(das33_distribute_pledge_operation(get_das33_administrator_id(), get_das33_pledges()[0].id, 10000, 10000, 500));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 3);

    // Change project phase
    das33_project_update_operation project_update_phase;
        project_update_phase.project_id = project.id;
        project_update_phase.authority  = get_das33_administrator_id();
        project_update_phase.phase_number = 1;
    do_op_no_balance_check(project_update_phase);

    // Pledge DASC in phase 1
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 4);

    // Complete first pledge: <  0%,   0%, 95% > and remove it
    do_op_no_balance_check(das33_distribute_pledge_operation(get_das33_administrator_id(), get_das33_pledges()[0].id, 0, 0, 9500));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 3);

    // Complete all phase 0 pledges
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 0, 10000, 10000, 10000));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 1);

    // Complete all phase 1 pledges
    do_op_no_balance_check(das33_distribute_project_pledges_operation(get_das33_administrator_id(), project.id, 1, 10000, 10000, 10000));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( das33_reject_project_test )
{ try {

    ACTOR(user);
    ACTOR(owner);
    VAULT_ACTOR(vault);

    tether_accounts(user_id, vault_id);

    // Issue a bunch of assets

    issue_dascoin(vault_id, 100);
    disable_vault_to_wallet_limit(vault_id);
    transfer_dascoin_vault_to_wallet(vault_id, user_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
    BOOST_CHECK_EQUAL( get_balance(user_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );

    // Create project tokens

    asset_id_type test_asset_id_0 = create_new_asset("ZERO", 100000000, 2, price({asset(1),asset(1,asset_id_type(1))})); // max_supply is 1.000.000,00000, precision is 2
    asset_id_type test_asset_id_1 = create_new_asset("ONE", 100000000, 2, price({asset(1),asset(1,asset_id_type(1))})); // max_supply is 1.000.000,00000, precision is 2
    asset_id_type test_asset_id_2 = create_new_asset("TWO", 100000000, 2, price({asset(1),asset(1,asset_id_type(1))})); // max_supply is 1.000.000,00000, precision is 2

    // Create das33 projects

    das33_project_create_operation project_create_0;
        project_create_0.authority       = get_das33_administrator_id();
        project_create_0.name            = "test_project_0";
        project_create_0.owner           = owner_id;
        project_create_0.token           = test_asset_id_0;
        project_create_0.discounts       = {{get_dascoin_asset_id(),100}};
        project_create_0.goal_amount_eur = 10000000;
        project_create_0.min_pledge      = 0;
        project_create_0.max_pledge      = 10000000;
    do_op(project_create_0);

    das33_project_create_operation project_create_1;
        project_create_1.authority       = get_das33_administrator_id();
        project_create_1.name            = "test_project_1";
        project_create_1.owner           = owner_id;
        project_create_1.token           = test_asset_id_1;
        project_create_1.discounts       = {{get_dascoin_asset_id(),100}};
        project_create_1.goal_amount_eur = 10000000;
        project_create_1.min_pledge      = 0;
        project_create_1.max_pledge      = 10000000;
    do_op(project_create_1);

    das33_project_create_operation project_create_2;
        project_create_1.authority       = get_das33_administrator_id();
        project_create_1.name            = "test_project_2";
        project_create_1.owner           = owner_id;
        project_create_1.token           = test_asset_id_2;
        project_create_1.discounts       = {{get_dascoin_asset_id(),100}};
        project_create_1.goal_amount_eur = 10000000;
        project_create_1.min_pledge      = 0;
        project_create_1.max_pledge      = 10000000;
    do_op(project_create_1);

    // Activate projects

    das33_project_object project_0 = get_das33_projects()[0];
    das33_project_object project_1 = get_das33_projects()[1];
    das33_project_object project_2 = get_das33_projects()[2];

    das33_project_update_operation project_update_0;
        project_update_0.project_id = project_0.id;
        project_update_0.authority  = get_das33_administrator_id();
        project_update_0.status     = das33_project_status::active;
    do_op(project_update_0);

    das33_project_update_operation project_update_1;
        project_update_1.project_id = project_1.id;
        project_update_1.authority  = get_das33_administrator_id();
        project_update_1.status     = das33_project_status::active;
    do_op(project_update_1);

    das33_project_update_operation project_update_2;
        project_update_2.project_id = project_2.id;
        project_update_2.authority  = get_das33_administrator_id();
        project_update_2.status     = das33_project_status::active;
    do_op(project_update_2);

    // Initial check
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

    // Pledge DASC
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_0.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_0.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_0.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_1.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_1.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_1.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_2.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_2.id));
    do_op_no_balance_check(das33_pledge_asset_operation(user_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, optional<license_type_id_type>{}, project_2.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 9);

    // Reject pledge from project 1
    do_op_no_balance_check(das33_pledge_reject_operation(get_das33_administrator_id(), get_das33_pledges()[4].id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 8);

    // Reject project 2
    do_op_no_balance_check(das33_project_reject_operation(get_das33_administrator_id(), project_2.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 5);

    // Reject project 0
    do_op_no_balance_check(das33_project_reject_operation(get_das33_administrator_id(), project_0.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 2);

    // Reject project 1
    do_op_no_balance_check(das33_project_reject_operation(get_das33_administrator_id(), project_1.id));
    BOOST_CHECK_EQUAL(get_das33_pledges().size(), 0);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests::das33_tests
BOOST_AUTO_TEST_SUITE_END()  // dascoin_tests

