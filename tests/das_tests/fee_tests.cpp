#include <boost/test/unit_test.hpp>

#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/issued_asset_record_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/app/database_api.hpp>
#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( fee_tests, database_fixture )

BOOST_AUTO_TEST_CASE( issued_cycle_asset_test )
{ try {
  ACTOR(wallet);

  const auto created_record = issue_cycleasset("CL1", wallet_id, 100, 100);
  BOOST_CHECK( created_record != nullptr );
  BOOST_CHECK_EQUAL( created_record->unique_id, "CL1" );
  BOOST_CHECK( created_record->issuer == get_webasset_issuer_id() );
  BOOST_CHECK( created_record->receiver == wallet_id );
  BOOST_CHECK( created_record->asset_type == get_cycle_asset_id() );
  BOOST_CHECK_EQUAL( created_record->amount.value, 100 );
  BOOST_CHECK_EQUAL( created_record->reserved.value, 100 );

  const auto fetched_record = _dal.get_issued_asset_record("CL1", get_cycle_asset_id());
  BOOST_CHECK( fetched_record.valid() );
  BOOST_CHECK_EQUAL( fetched_record->unique_id, "CL1" );
  BOOST_CHECK( fetched_record->issuer == get_webasset_issuer_id() );
  BOOST_CHECK( fetched_record->receiver == wallet_id );
  BOOST_CHECK( fetched_record->asset_type == get_cycle_asset_id() );
  BOOST_CHECK_EQUAL( fetched_record->amount.value, 100 );
  BOOST_CHECK_EQUAL( fetched_record->reserved.value, 100 );

  GRAPHENE_REQUIRE_THROW( issue_cycleasset("CL1", wallet_id, 100, 100), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( check_issued_cycle_test )
{ try {
  ACTOR(wallet);

  auto created_record = issue_cycleasset("CL1", wallet_id, 100, 100);
  BOOST_CHECK( created_record != nullptr );
  bool found = _dal.check_issued_asset("CL1", DASCOIN_CYCLE_ASSET_SYMBOL);
  BOOST_CHECK( found );

  // This was issued also:
  found = _dal.check_issued_asset("CL1", DASCOIN_CYCLE_ASSET_SYMBOL);
  BOOST_CHECK( found );

  // Issue another one, different unique id:
  created_record = issue_cycleasset("CL2", wallet_id, 100, 100);
  BOOST_CHECK( created_record != nullptr );

  // The first one should still be reachable:
  found = _dal.check_issued_asset("CL1", DASCOIN_CYCLE_ASSET_SYMBOL);
  BOOST_CHECK( found );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( successful_fee_change_and_fee_charge_with_fee_burn_test )
{ try {

   auto root_id = db.get_global_properties().authorities.root_administrator;

   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   change_operation_fee_operation cffo;
   cffo.issuer = root_id;
   cffo.new_fee = 30;
   cffo.op_num = 1;

   do_op(cffo);

   ACTOR(alice);
   ACTOR(bob);
   VAULT_ACTOR(bobv);
   VAULT_ACTOR(alicev);

   tether_accounts(alice_id, alicev_id);
   tether_accounts(bob_id, bobv_id);

   do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), bobv_id, 200, 200, ""));
   toggle_reward_queue(true);

   // Wait for the cycles to be distributed:
   generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

   // Set limit to 100 dascoin
   db.adjust_balance_limit(bobv, get_dascoin_asset_id(), 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

   transfer_dascoin_vault_to_wallet(bobv_id, bob_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
   issue_webasset("1", alice_id, 100, 100);
   generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

   share_type cash, reserved;
   std::tie(cash, reserved) = get_web_asset_amounts(alice_id);

   // 100 cash, 100 reserved
   BOOST_CHECK_EQUAL( cash.value, 100 );
   BOOST_CHECK_EQUAL( reserved.value, 100 );

   set_expiration( db, trx );

   issue_cycleasset("CL1", alice_id, 100, 100);

   auto loco = limit_order_create_operation(alice_id, asset{0, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 100,
                                             alicev_id, db.head_block_time() + fc::seconds(60));
   loco.fee = asset{30, get_cycle_asset_id()};

   do_op(loco);

   create_sell_order(bob_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, asset{100, get_web_asset_id()});

   std::tie(cash, reserved) = get_web_asset_amounts(alice_id);

   // test db_api->get_required_fees
   graphene::app::database_api db_api(db);
   vector<operation> ops;
   ops.push_back(limit_order_create_operation());

   vector<variant> fees = db_api.get_required_fees(ops, get_cycle_asset_id());

   for(auto v : fees)
   {
	   asset a = v.as<asset>();
	   BOOST_CHECK_EQUAL( a.amount.value, 30 );
   }

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( successful_pool_account_change_and_fee_charge_test )
{ try {

   auto root_id = db.get_global_properties().authorities.root_administrator;

   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   change_operation_fee_operation cffo;
   cffo.issuer = root_id;
   cffo.new_fee = 30;
   cffo.op_num = 1;

   do_op(cffo);

   ACTOR(alice);
   ACTOR(bob);
   VAULT_ACTOR(bobv);
   VAULT_ACTOR(alicev);

   tether_accounts(alice_id, alicev_id);
   tether_accounts(bob_id, bobv_id);

   do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), bobv_id, 200, 200, ""));
   toggle_reward_queue(true);

   // Wait for the cycles to be distributed:
   generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

   // Set limit to 100 dascoin
   db.adjust_balance_limit(bobv, get_dascoin_asset_id(), 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

   transfer_dascoin_vault_to_wallet(bobv_id, bob_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
   issue_webasset("1", alice_id, 100, 100);
   generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

   share_type cash, reserved;
   std::tie(cash, reserved) = get_web_asset_amounts(alice_id);

   // 100 cash, 100 reserved
   BOOST_CHECK_EQUAL( cash.value, 100 );
   BOOST_CHECK_EQUAL( reserved.value, 100 );

   set_expiration( db, trx );

   issue_cycleasset("CL1", alice_id, 100, 100);

   // set pool account
   ACTOR(pool);
   // this will make cycle balance object for pool account
   issue_cycleasset("pool", pool_id, 10, 10);

   change_fee_pool_account_operation cfpao;
   cfpao.issuer = root_id;
   cfpao.fee_pool_account_id = pool_id;
   do_op(cfpao);


   auto loco = limit_order_create_operation(alice_id, asset{0, get_web_asset_id()}, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, 100,
                                             alicev_id, db.head_block_time() + fc::seconds(60));
   loco.fee = asset{30 , get_cycle_asset_id()};

   do_op(loco);

   create_sell_order(bob_id, asset{100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()}, asset{100, get_web_asset_id()});

   auto balance = get_balance(pool_id, get_cycle_asset_id());

   BOOST_CHECK_EQUAL( balance, 70 + DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( successful_pool_account_change_and_cycle_submit_test )
{ try {
   auto root_id = db.get_global_properties().authorities.root_administrator;
   BOOST_CHECK( db.get_dynamic_global_properties().is_root_authority_enabled_flag );

   toggle_reward_queue(true);

   // set pool account
   ACTOR(pool);
   // this will make cycle balance object for pool account
   issue_cycleasset("pool", pool_id, 100, 0);

   change_fee_pool_account_operation cfpao;
   cfpao.issuer = root_id;
   cfpao.fee_pool_account_id = pool_id;
   do_op(cfpao);

   fee_pool_cycles_submit_operation fpcso;
   fpcso.issuer = pool_id;
   fpcso.amount = 40;
   do_op(fpcso);

   // Wait for the cycles to be distributed:
   generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

   auto cycle_balance = get_balance(pool_id, get_cycle_asset_id());

   BOOST_CHECK_EQUAL( cycle_balance, 60 + DASCOIN_DEFAULT_STARTING_CYCLE_ASSET_AMOUNT);

   auto dasc_balance = get_balance(pool_id, get_dascoin_asset_id());

   BOOST_CHECK_EQUAL( dasc_balance, 2000000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
