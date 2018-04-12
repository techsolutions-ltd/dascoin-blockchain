/**
 * DASCOIN!
 */

#include <boost/test/unit_test.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/access_layer.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/queue_objects.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( access_layer_unit_tests, database_fixture )

BOOST_AUTO_TEST_CASE( get_free_cycle_balance_non_existant_id_unit_test )
{ try {
  auto bogus_id = account_id_type(999999);

  auto res = _dal.get_free_cycle_balance(bogus_id);
  BOOST_CHECK( res.account_id == bogus_id );
  BOOST_CHECK( !res.result.valid() );
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_free_cycle_balance_unit_test )
{ try {
  VAULT_ACTOR(vault)

  auto res = _dal.get_free_cycle_balance(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  BOOST_CHECK_EQUAL( res.result->value, 0 );

  adjust_cycles(vault_id, 100);

  res = _dal.get_free_cycle_balance(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  BOOST_CHECK_EQUAL( res.result->value, 100 );

  adjust_cycles(vault_id, 300);

  res = _dal.get_free_cycle_balance(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  BOOST_CHECK_EQUAL( res.result->value, 400 );

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, 200, 200, ""));

  res = _dal.get_free_cycle_balance(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  BOOST_CHECK_EQUAL( res.result->value, 400 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_all_cycle_balances_non_existant_id_unit_test )
{ try {
  auto bogus_id = account_id_type(999999);

  auto res = _dal.get_all_cycle_balances(bogus_id);
  BOOST_CHECK( res.account_id == bogus_id );
  BOOST_CHECK( !res.result.valid() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_all_cycle_balances_unit_test )
{ try {
  VAULT_ACTOR(vault)

  adjust_cycles(vault_id, 100);

  auto res = _dal.get_all_cycle_balances(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  auto cycle_vec = *res.result;
  BOOST_CHECK_EQUAL( cycle_vec.size(), 1 );
  BOOST_CHECK_EQUAL( cycle_vec[0].cycles.value, 100 );
  BOOST_CHECK_EQUAL( cycle_vec[0].frequency_lock.value, 0 );

  adjust_cycles(vault_id, 1000);
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, 210, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, 220, 200, ""));

  res = _dal.get_all_cycle_balances(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  cycle_vec = *res.result;
  BOOST_CHECK_EQUAL( cycle_vec.size(), 4 );
  BOOST_CHECK_EQUAL( cycle_vec[0].cycles.value, 1100 );
  BOOST_CHECK_EQUAL( cycle_vec[0].frequency_lock.value, 0 );
  BOOST_CHECK_EQUAL( cycle_vec[1].cycles.value, 200 );
  BOOST_CHECK_EQUAL( cycle_vec[1].frequency_lock.value, 200 );
  BOOST_CHECK_EQUAL( cycle_vec[2].cycles.value, 210 );
  BOOST_CHECK_EQUAL( cycle_vec[2].frequency_lock.value, 200 );
  BOOST_CHECK_EQUAL( cycle_vec[3].cycles.value, 220 );
  BOOST_CHECK_EQUAL( cycle_vec[3].frequency_lock.value, 200 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_dascoin_balance_non_existant_id_unit_test )
{ try {
  auto bogus_id = account_id_type(999999);

  auto res = _dal.get_dascoin_balance(bogus_id);
  BOOST_CHECK( res.account_id == bogus_id );
  BOOST_CHECK( !res.result.valid() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_dascoin_balances )
{ try {
  VAULT_ACTOR(vault)

  db.issue_asset(vault_id, 1000 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id(), 0);

  auto res = _dal.get_dascoin_balance(vault_id);
  BOOST_CHECK( res.account_id == vault_id );
  BOOST_CHECK_EQUAL( res.result->value, 1000 * DASCOIN_DEFAULT_ASSET_PRECISION );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_free_cycle_balances_for_accounts_unit_test )
{ try {
  VAULT_ACTORS((first)(second)(third))
  auto bogus_id = account_id_type(99999999);

  adjust_cycles(second_id, 200);
  adjust_cycles(third_id, 300);

  auto res_vec = _dal.get_free_cycle_balances_for_accounts({first_id, second_id, third_id, bogus_id});

  BOOST_CHECK_EQUAL( res_vec.size(), 4 );

  BOOST_CHECK( res_vec[0].account_id == first_id );
  BOOST_CHECK_EQUAL( res_vec[0].result->value, 0 );

  BOOST_CHECK( res_vec[1].account_id == second_id );
  BOOST_CHECK_EQUAL( res_vec[1].result->value, 200 );

  BOOST_CHECK( res_vec[2].account_id == third_id );
  BOOST_CHECK_EQUAL( res_vec[2].result->value, 300 );

  BOOST_CHECK( res_vec[3].account_id == bogus_id );
  BOOST_CHECK( !res_vec[3].result.valid() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_all_cycle_balances_for_accounts_unit_test )
{ try {
  VAULT_ACTORS((zero)(somefree)(somequeue)(both))
  auto bogus_id = account_id_type(99999);

  adjust_cycles( somefree_id, 100 );
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), somequeue_id, 200, 200, ""));
  adjust_cycles( both_id, 300 );
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), both_id, 310, 200, ""));

  auto result_vec = _dal.get_all_cycle_balances_for_accounts({zero_id, somefree_id, somequeue_id, both_id, bogus_id});

  BOOST_CHECK_EQUAL( result_vec.size(), 5 );

  // Zero has balance of 0 with lock 0:
  BOOST_CHECK( result_vec[0].account_id == zero_id );
  auto agreement_vec = *result_vec[0].result;
  BOOST_CHECK_EQUAL( agreement_vec.size(), 1 );
  BOOST_CHECK_EQUAL( agreement_vec[0].cycles.value, 0 );
  BOOST_CHECK_EQUAL( agreement_vec[0].frequency_lock.value, 0 );

  // Somefree has balance of 100 with lock 0:
  BOOST_CHECK( result_vec[1].account_id == somefree_id );
  agreement_vec = *result_vec[1].result;
  BOOST_CHECK_EQUAL( agreement_vec.size(), 1 );
  BOOST_CHECK_EQUAL( agreement_vec[0].cycles.value, 100 );
  BOOST_CHECK_EQUAL( agreement_vec[0].frequency_lock.value, 0 );

  // Somequeue has balance of 0 with frequency lock 0 and balance 200 with lock 200:
  BOOST_CHECK( result_vec[2].account_id == somequeue_id );
  agreement_vec = *result_vec[2].result;
  BOOST_CHECK_EQUAL( agreement_vec.size(), 2 );
  BOOST_CHECK_EQUAL( agreement_vec[0].cycles.value, 0 );
  BOOST_CHECK_EQUAL( agreement_vec[0].frequency_lock.value, 0 );
  BOOST_CHECK_EQUAL( agreement_vec[1].cycles.value, 200 );
  BOOST_CHECK_EQUAL( agreement_vec[1].frequency_lock.value, 200 );

  // Both has balance of 300 with lock 0 and balance of 310 with lock 200:
  BOOST_CHECK( result_vec[3].account_id == both_id );
  agreement_vec = *result_vec[3].result;
  BOOST_CHECK_EQUAL( agreement_vec.size(), 2 );
  BOOST_CHECK_EQUAL( agreement_vec[0].cycles.value, 300 );
  BOOST_CHECK_EQUAL( agreement_vec[0].frequency_lock.value, 0 );
  BOOST_CHECK_EQUAL( agreement_vec[1].cycles.value, 310 );
  BOOST_CHECK_EQUAL( agreement_vec[1].frequency_lock.value, 200 );

  // Bogus does not have a result:
  BOOST_CHECK( result_vec[4].account_id == bogus_id );
  BOOST_CHECK( !result_vec[4].result.valid() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( lookup_asset_symbol_unit_test )
{ try {
  const auto& symbol = _dal.lookup_asset_symbol("1.3.1");
  BOOST_CHECK( symbol.valid() );

  const auto& symbol2 = _dal.lookup_asset_symbol("WEBEUR");
  BOOST_CHECK( symbol2.valid() );

  // Non existing assets here:
  const auto& symbol3 = _dal.lookup_asset_symbol("FOO");
  BOOST_CHECK( !symbol3.valid() );

  const auto& symbol4 = _dal.lookup_asset_symbol("1.3.4");
  BOOST_CHECK( !symbol4.valid() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( lookup_asset_symbols_unit_test )
{ try {
  vector<string> symbols_and_ids{"1.3.1", "WEBEUR", "1.3.2", "FOO", "DAS"};
  const auto& symbols = _dal.lookup_asset_symbols(symbols_and_ids);

  // There should be 5 items:
  BOOST_CHECK_EQUAL( symbols.size(), 5 );
  BOOST_CHECK( symbols[0].valid() );
  BOOST_CHECK( symbols[1].valid() );
  BOOST_CHECK( symbols[2].valid() );
  BOOST_CHECK( !symbols[3].valid() );
  BOOST_CHECK( symbols[4].valid() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_vaults_info_unit_test )
{ try {
  ACTOR(wallet)
  VAULT_ACTOR(vault1)
  VAULT_ACTOR(vault2)
  VAULT_ACTOR(vault3)

  auto bogus_id = account_id_type(999999);

  auto w = _dal.get_vault_info(vault1_id);
  auto v1 = _dal.get_vault_info(vault1_id);
  auto v2 = _dal.get_vault_info(vault2_id);
  auto v3 = _dal.get_vault_info(vault3_id);

  auto res = _dal.get_vaults_info({bogus_id, wallet_id, vault1_id, vault2_id, vault3_id});

  BOOST_CHECK_EQUAL( res.size(), 5 );

  BOOST_CHECK( res[0].account_id == bogus_id );
  BOOST_CHECK( !res[0].result.valid() );

  BOOST_CHECK( res[1].account_id == wallet_id );
  BOOST_CHECK( !res[1].result.valid() );

  BOOST_CHECK( res[2].account_id == vault1_id );
  BOOST_CHECK( res[2].result.valid() );
  BOOST_CHECK( res[2].result->eur_limit == v1->eur_limit );

} FC_LOG_AND_RETHROW () }

BOOST_AUTO_TEST_CASE( account_tethered_unit_test )
{ try {
  ACTOR(wallet);
  VAULT_ACTOR(vault);

  auto resv = _dal.get_vault_info(vault_id);

  BOOST_CHECK( resv.valid() );
  BOOST_CHECK( !resv->is_tethered );
  BOOST_CHECK( !wallet.is_tethered() );

  tether_accounts(wallet_id, vault_id);
  resv = _dal.get_vault_info(vault_id);

  BOOST_CHECK( resv->is_tethered );
  BOOST_CHECK( wallet.is_tethered() );
  BOOST_CHECK( wallet.is_tethered_to(vault_id) );
  BOOST_CHECK( vault.is_tethered_to(wallet_id) );

} FC_LOG_AND_RETHROW () }

BOOST_AUTO_TEST_CASE( get_blocks_test )
{ try {
  VAULT_ACTOR(vault);

  // const auto& check_blocks = [&](uint32_t start, uint32_t count)
  // { try {
  //   const auto results = _dal.get_blocks(start, count);
  //   BOOST_CHECK_EQUAL(results.size(), count);
  //   for (uint32_t i = 0; i < count; ++i) {
  //     const auto res = results[i];
  //     BOOST_CHECK_EQUAL(res.num, i+start);
  //   }
  // } FC_LOG_AND_RETHROW() };

  // First, set up the blocks:
  for(int i = 0; i < 20; ++i) {
    do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), vault_id, (i+1)*100, 200, "TEST"));
  }
  // NOTE: two blocks are generated during test set up.
  BOOST_CHECK_EQUAL(db.head_block_num(), 22);

  // ERROR: start block is higher than head block number.
  GRAPHENE_REQUIRE_THROW(_dal.get_blocks(55, 2), fc::exception);
  
  // ERROR: number of blocks to fetch is 0.
  GRAPHENE_REQUIRE_THROW(_dal.get_blocks(4, 0), fc::exception);

  vector<signed_block_with_num> results;

  // Starting from block #2, fetch the following 20 blocks:
  results = _dal.get_blocks(2, 20);
  BOOST_CHECK_EQUAL(results.size(), 20);
  for (int i = 0; i < 20; ++i) {
    const auto res = results[i];
    // We are fetching blocks [2,22]:
    BOOST_CHECK_EQUAL(res.num, i+2);
  }

  // Starting from block #18, fetch the following 20 blocks:
  results = _dal.get_blocks(18, 20);
  // Only 4 blocks are available:
  BOOST_CHECK_EQUAL(results.size(), 4);
  for (int i = 0; i < 4; ++i) {
    const auto res = results[i];
    // We are fetching blocks [18,22]:
    BOOST_CHECK_EQUAL(res.num, i+18);
  }

  // Test if block id is correct:
  const auto block_id = _dal.get_blocks(18, 1)[0].block.id();
  const auto stored_id = _dal.get_blocks(18, 1)[0].block_id;
  BOOST_CHECK(block_id == stored_id);

} FC_LOG_AND_RETHROW() }

/*BOOST_AUTO_TEST_CASE( get_all_cycle_balances_for_accounts_unit_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  auto vec = _dal.get_all_cycle_balances_for_accounts({});

  BOOST_CHECK_EQUAL( vec.size(), 0 );

  adjust_cycles( first_id, 100 );
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 110, 200, ""));

  adjust_cycles( second_id, 200 );

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 300, 200, ""));

  vec = _dal.get_all_cycle_balances_for_accounts({first_id, second_id, third_id, fourth_id});

  BOOST_CHECK_EQUAL( vec.size(), 4 );

  auto cycle_vec = vec[0];
  BOOST_CHECK_EQUAL( cycle_vec.size(), 2 );
  BOOST_CHECK_EQUAL( cycle_vec[0].cycles.value, 100 );
  BOOST_CHECK_EQUAL( cycle_vec[0].frequency_lock.value, 0 );
  BOOST_CHECK_EQUAL( cycle_vec[1].cycles.value, 110 );
  BOOST_CHECK_EQUAL( cycle_vec[1].frequency_lock.value, 200 );

  cycle_vec = vec[1];
  BOOST_CHECK_EQUAL( cycle_vec.size(), 1 );
  BOOST_CHECK_EQUAL( cycle_vec[0].cycles.value, 200 );

  cycle_vec = vec[2];
  BOOST_CHECK_EQUAL( cycle_vec.size(), 2 );
  BOOST_CHECK_EQUAL( cycle_vec[0].cycles.value, 0 );
  BOOST_CHECK_EQUAL( cycle_vec[0].frequency_lock.value, 0 );
  BOOST_CHECK_EQUAL( cycle_vec[1].cycles.value, 300 );
  BOOST_CHECK_EQUAL( cycle_vec[1].frequency_lock.value, 200 );

  cycle_vec = vec[3];
  BOOST_CHECK_EQUAL( cycle_vec.size(), 1 );
  BOOST_CHECK_EQUAL( cycle_vec[0].cycles.value, 0 );
  BOOST_CHECK_EQUAL( cycle_vec[0].frequency_lock.value, 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_dascoin_balances_for_accounts_unit_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  auto das_vec = _dal.get_dascoin_balances_for_accounts({});

  BOOST_CHECK_EQUAL( das_vec.size(), 0 );

  db.issue_asset(first_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id(), 0);
  db.issue_asset(second_id, 200 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id(), 0);
  db.issue_asset(third_id, 300 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id(), 0);

  das_vec = _dal.get_dascoin_balances_for_accounts({first_id, second_id, third_id, fourth_id});

  BOOST_CHECK_EQUAL( das_vec.size(), 4 );
  BOOST_CHECK_EQUAL( das_vec[0].value, 100 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( das_vec[1].value, 200 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( das_vec[2].value, 300 * DASCOIN_DEFAULT_ASSET_PRECISION );
  BOOST_CHECK_EQUAL( das_vec[3].value, 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_queue_submissions_with_pos_unit_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 100, 200, "test1"));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 120, 200, ""));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 210, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 220, 200, ""));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 300, 200, ""));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), fourth_id, 400, 200, ""));

  auto pos_vec = _dal.get_queue_submissions_with_pos(first_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 2 );

  uint32_t pos;
  reward_queue_object rqo;
  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 0 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 100 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );
  // BOOST_CHECK( rqo.time == db.head_block_time() ); // TODO: this fails for some reason?
  BOOST_CHECK( rqo.comment == "test1" );

  std::tie(pos, rqo) = pos_vec[1];
  BOOST_CHECK_EQUAL( pos, 1 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 120 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = _dal.get_queue_submissions_with_pos(second_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 3 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 2 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 200 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  std::tie(pos, rqo) = pos_vec[1];
  BOOST_CHECK_EQUAL( pos, 3 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 210 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  std::tie(pos, rqo) = pos_vec[2];
  BOOST_CHECK_EQUAL( pos, 4 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 220 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = _dal.get_queue_submissions_with_pos(third_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 5 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == third_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 300 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = _dal.get_queue_submissions_with_pos(fourth_id);
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 6 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == fourth_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 400 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_queue_submissions_with_pos_for_accounts_unit_test )
{ try {
  VAULT_ACTORS((first)(second)(third)(fourth))
  uint32_t pos;
  reward_queue_object rqo;

  auto vec = _dal.get_queue_submissions_with_pos_for_accounts({});
  BOOST_CHECK_EQUAL( vec.size(), 0 );

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), first_id, 100, 200, ""));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 200, 200, ""));
  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), second_id, 210, 200, ""));

  do_op(submit_reserve_cycles_to_queue_operation(get_cycle_issuer_id(), third_id, 300, 200, ""));

  vec = _dal.get_queue_submissions_with_pos_for_accounts({first_id, second_id, third_id, fourth_id});
  BOOST_CHECK_EQUAL( vec.size(), 4 );

  auto pos_vec = vec[0];
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 0 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == first_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 100 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );
  // BOOST_CHECK( rqo.time == db.head_block_time() ); // TODO: this fails for some reason?

  pos_vec = vec[1];
  BOOST_CHECK_EQUAL( pos_vec.size(), 2 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 1 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 200 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  std::tie(pos, rqo) = pos_vec[1];
  BOOST_CHECK_EQUAL( pos, 2 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == second_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 210 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = vec[2];
  BOOST_CHECK_EQUAL( pos_vec.size(), 1 );

  std::tie(pos, rqo) = pos_vec[0];
  BOOST_CHECK_EQUAL( pos, 3 );
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK( !rqo.license.valid() );
  BOOST_CHECK( rqo.account == third_id );
  BOOST_CHECK_EQUAL( rqo.amount.value, 300 );
  BOOST_CHECK_EQUAL( rqo.frequency.value, 200 );

  pos_vec = vec[3];
  BOOST_CHECK_EQUAL( pos_vec.size(), 0 );

} FC_LOG_AND_RETHROW() }*/

BOOST_AUTO_TEST_CASE( get_block_with_virtual_operations )
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

    const auto issue_assets = [&, this](share_type web_assets, share_type web_assets_reserved, share_type expected_web_assets, share_type web_assets_reserved_expected)
    {
        set_expiration( db, trx );
        issue_webasset("1", alice_id, web_assets, web_assets_reserved);

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

    // Set limit to 100 dascoin
    db.adjust_balance_limit(bob, get_dascoin_asset_id(), 100 * DASCOIN_DEFAULT_ASSET_PRECISION);

    transfer_dascoin_vault_to_wallet(bob_id, bobw_id, 100 * DASCOIN_DEFAULT_ASSET_PRECISION);
    transfer_webasset_vault_to_wallet(alice_id, alicew_id, {1000, 100});

    // at this point, alice got 1000+100 web assets and bobw got 100 dascoins

    set_expiration( db, trx );

    // place two orders which will produce a match
    create_sell_order(alicew_id, asset{1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()},
                      asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()});
    create_sell_order(bobw_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()},
                      asset{1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()});

    // balances: alice 900+100, bob 0
    check_balances(alicew, 900, 100);
    BOOST_CHECK_EQUAL( get_balance(bobw_id, get_dascoin_asset_id()), 90 * DASCOIN_DEFAULT_ASSET_PRECISION );

    const auto &dgpo = get_dynamic_global_properties();
    const auto &dprice = dgpo.last_dascoin_price;
    const price expected_price{ asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()},
                                asset{1 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()} };
    BOOST_CHECK( dprice == expected_price );

    create_sell_order(alicew_id, asset{2 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()},
                      asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()});
    create_sell_order(bobw_id, asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()},
                      asset{2 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()});
    const auto &dgpo2 = get_dynamic_global_properties();
    const auto &dprice2 = dgpo2.last_dascoin_price;
    const price expected_price2{ asset{10 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id()},
                                 asset{2 * DASCOIN_FIAT_ASSET_PRECISION, get_web_asset_id()}};
    BOOST_CHECK( dprice2 == expected_price2 );

    generate_blocks(db.head_block_time() + fc::hours(24) + fc::seconds(1));

    vector<signed_block_with_virtual_operations_and_num> results;

    vector<uint16_t> virtual_op_ids;
    operation opad = record_distribute_dascoin_operation();
    operation opfo = fill_order_operation();

    virtual_op_ids.push_back(opfo.which());
    virtual_op_ids.push_back(opad.which());
    // Starting from block #2, fetch the following 20 blocks:
    results = _dal.get_blocks_with_virtual_operations(1, 20,virtual_op_ids);


    int count_vops = 0;
    for(auto& blc : results)
    {
       if(blc.block.virtual_operations.size() > 0)
       {
           for(operation& op : blc.block.virtual_operations)
           {
              count_vops++;
              BOOST_CHECK( (op.which() == opfo.which() || op.which() == opad.which()) );

//              string s = fc::json::to_string( op );
//              std::cout << s;
//              BOOST_CHECK( true );
           }
       }
    }

    BOOST_CHECK( count_vops == 5 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
