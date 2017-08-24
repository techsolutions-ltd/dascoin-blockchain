/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/market_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( exchange_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( order_not_enough_assets_test )
{ try {
    ACTOR(alice);

    issue_webasset(alice_id, 100, 100);
    generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));
    BOOST_CHECK_EQUAL( get_balance(alice_id, get_web_asset_id()), 100 );
    set_expiration( db, trx );

    // make a huge order that ought to fail
    // fixme: this is wrong
    GRAPHENE_REQUIRE_THROW(create_sell_order(alice_id, asset{1000, get_web_asset_id()}, asset{100, get_dascoin_asset_id()}), fc::exception);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( cancel_order_test )
{ try {
    ACTOR(alice);

    issue_webasset(alice_id, 100, 100);
    generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

    share_type cash, reserved;
    std::tie(cash, reserved) = get_web_asset_amounts(alice_id);

    // 100 cash, 0 reserved
    FC_ASSERT( cash == 100, "Invalid amount" );
    FC_ASSERT( reserved == 100, "Invalid amount" );

    set_expiration( db, trx );

    // make an order, check balance, then cancel the order
    auto order = create_sell_order(alice_id, asset{100, get_web_asset_id()}, asset{100, get_dascoin_asset_id()});
    BOOST_CHECK_EQUAL( get_balance(alice_id, get_web_asset_id()), 0 );
    cancel_limit_order(*order);
    BOOST_CHECK_EQUAL( get_balance(alice_id, get_web_asset_id()), 100 );

    set_expiration( db, trx );
    order = create_sell_order(alice_id, asset{0, get_web_asset_id()}, asset{100, get_dascoin_asset_id()}, 100);
    std::tie(cash, reserved) = get_web_asset_amounts(alice_id);

    // 100 cash, 0 reserved
    BOOST_CHECK_EQUAL( cash.value, 100 );
    BOOST_CHECK_EQUAL( reserved.value, 0 );

    cancel_limit_order(*order);
    std::tie(cash, reserved) = get_web_asset_amounts(alice_id);

    // 100 cash, 100 reserved
    BOOST_CHECK_EQUAL( cash.value, 100 );
    BOOST_CHECK_EQUAL( reserved.value, 100 );

    set_expiration( db, trx );
    // make an order, let it expire
    do_op(limit_order_create_operation(alice_id, asset{0, get_web_asset_id()}, asset{100, get_dascoin_asset_id()}, 100,
                                       optional<account_id_type>(), db.head_block_time() + fc::seconds(60)));

    generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);
    generate_block();

    std::tie(cash, reserved) = get_web_asset_amounts(alice_id);
    // 100 cash, 100 reserved
    BOOST_CHECK_EQUAL( cash.value, 100 );
    BOOST_CHECK_EQUAL( reserved.value, 100 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( exchange_test )
{ try {
    ACTOR(alicew);
    ACTOR(bobw);
    VAULT_ACTOR(bob);
    VAULT_ACTOR(alice);

    const auto check_balances = [this](const account_object& account, share_type expected_cash,
                                       share_type expected_reserved)
    {
        share_type cash, reserved;
        std::tie(cash, reserved) = get_web_asset_amounts(account.id);
        bool amount_ok = (cash == expected_cash && reserved == expected_reserved);
        FC_ASSERT( amount_ok, "On account '${n}': balance = (${c}/${r}), expected = (${ec}/${er})",
                   ("n", account.name)("c", cash)("r", reserved)("ec", expected_cash)("er", expected_reserved));
    };

    const auto issue_request = [this](const account_object& account, share_type cash, share_type reserved)
    {
        const issue_asset_request_object* p = issue_webasset(account.id, cash, reserved);
        FC_ASSERT( p, "Asset request object for '${n}' was not created",("n",account.name));
        // ilog( "Creating asset request for '${n}'", ("n", (p->receiver)(db).name));
        FC_ASSERT( p->issuer == get_webasset_issuer_id() );
        FC_ASSERT( p->receiver == account.id, "Receiver: '${n}', Account: '${an}'", ("n", (p->receiver)(db).name)("an", account.name));
        FC_ASSERT( p->amount == cash );
        FC_ASSERT( p->asset_id == get_web_asset_id() );
        FC_ASSERT( p->reserved_amount == reserved );
    };

    const auto issue_assets = [&, this](share_type web_assets, share_type web_assets_reserved, share_type expected_web_assets, share_type web_assets_reserved_expected)
    {
        set_expiration( db, trx );
        issue_request(alice, web_assets, web_assets_reserved);

        adjust_dascoin_reward(500 * DASCOIN_DEFAULT_ASSET_PRECISION);
        adjust_frequency(200);

        do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), bob_id, 200, 200, ""));
        toggle_reward_queue(true);

        // Wait for the cycles to be distributed:
        generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));
        check_balances(alice, expected_web_assets, web_assets_reserved_expected);
        BOOST_CHECK_EQUAL( get_balance(bob_id, get_dascoin_asset_id()), 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
    };

    issue_assets(1000, 100, 1000, 100);

    tether_accounts(bobw_id, bob_id);
    tether_accounts(alicew_id, alice_id);
    transfer_dascoin_vault_to_wallet(bob_id, bobw_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
    transfer_webasset_vault_to_wallet(alice_id, alicew_id, {1000, 100});

    // at this point, alice got 1000+100 web assets and bobw got 100 dascoins

    set_expiration( db, trx );

    // place two orders which will produce a match
    create_sell_order(alicew_id, asset{100, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()});
    create_sell_order(bobw_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, asset{100, get_web_asset_id()});

    // balances: alice 900+100, bob 0
    check_balances(alicew, 900, 100);
    BOOST_CHECK_EQUAL( get_balance(bobw_id, get_dascoin_asset_id()), 0 );

    // partial match
//    issue_assets(100, 0, 100, 0);
//    transfer_dascoin_vault_to_wallet(bob_id, bobw_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
//    transfer_webasset_vault_to_wallet(alice_id, alicew_id, {1000, 100});
//
//    set_expiration( db, trx );
//    auto order_id = create_sell_order(alicew_id, asset{100, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()});
//    create_sell_order(bobw_id, asset{50 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, asset{50, get_web_asset_id()});
//    cancel_limit_order(*order_id);

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
