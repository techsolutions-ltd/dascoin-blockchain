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

#include <graphene/chain/protocol/license.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/frequency_history_record_object.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/upgrade_event_object.hpp>

#include "../common/database_fixture.hpp"

using namespace graphene::chain;
using namespace graphene::chain::test;

BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( license_tests, database_fixture )

BOOST_AUTO_TEST_CASE( regression_test_license_information_index )
{ try {

  db.create<license_information_object>([&](license_information_object& lio){});
  db.create<license_information_object>([&](license_information_object& lio){});

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( charter_license_type_value_test )
{ try {

  auto lic = *(_dal.get_license_type("standard_charter"));
  BOOST_CHECK_EQUAL( lic.amount.value, DASCOIN_BASE_STANDARD_CYCLES );

  lic = *(_dal.get_license_type("manager_charter"));
  BOOST_CHECK_EQUAL( lic.amount.value, DASCOIN_BASE_MANAGER_CYCLES );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( license_information_unit_test )
{ try {
  VAULT_ACTOR(vault);

  auto standard_charter = *(_dal.get_license_type("standard_charter"));
  const share_type bonus_percent = 50;
  const share_type frequency_lock = 200;
  const time_point_sec issue_time = db.head_block_time();
  const uint32_t amount = DASCOIN_BASE_STANDARD_CYCLES + (50 * DASCOIN_BASE_STANDARD_CYCLES) / 100;

  do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard_charter.id,
        bonus_percent, frequency_lock, issue_time));

  BOOST_CHECK( vault.license_information.valid() );

  const auto& license_information_obj = (*vault.license_information)(db);

  BOOST_CHECK( license_information_obj.account == vault_id );
  
  const auto& license_history = license_information_obj.history;

  BOOST_CHECK_EQUAL( license_history.size(), 1 );

  const auto& license_record = license_history[0];

  BOOST_CHECK( license_record.license == standard_charter.id );
  BOOST_CHECK_EQUAL( license_record.amount.value, amount );
  BOOST_CHECK_EQUAL( license_record.base_amount.value, DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( license_record.bonus_percent.value, 50 );
  BOOST_CHECK_EQUAL( license_record.frequency_lock.value, 200 );
  BOOST_CHECK( license_record.activated_at == issue_time );
  BOOST_CHECK( license_record.issued_on_blockchain == issue_time );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_license_types_unit_test )
{ try {

  auto lic_vec = _dal.get_license_types();

  // Number of kinds (regular, chartered, locked) times number of types (standard, manager, pro, executive, vise president, president + 1:
  BOOST_CHECK_EQUAL( lic_vec.size(), 19 );
  
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_license_type_names_ids_unit_test )
{ try {

  auto lic_vec = _dal.get_license_types();
  auto names_ids = _dal.get_license_type_names_ids();

  BOOST_CHECK_EQUAL( lic_vec.size(), names_ids.size() );

  for (size_t i = 0; i < names_ids.size(); ++i)
  {
    BOOST_CHECK_EQUAL( names_ids[i].first, lic_vec[i].name );
    BOOST_CHECK( names_ids[i].second == lic_vec[i].id );
  }

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( edit_license_type_test )
{ try {

  const auto& lic = _dal.get_license_type("standard");
  BOOST_CHECK(lic.valid());
  const auto& lic_id = lic->id;

  // Try to set license's name to a name which is longer than 64 chars:
  GRAPHENE_REQUIRE_THROW( do_op(edit_license_type_operation(get_license_administrator_id(), lic_id,
                                 "0123456789012345678901234567890123456789012345678901234567890123456789", 200, 10)), fc::exception );

  // Try to set license's name to an empty string:
  GRAPHENE_REQUIRE_THROW( do_op(edit_license_type_operation(get_license_administrator_id(), lic_id, "", 200, 10)), fc::exception );

  // Try to set license's amount to 0:
  GRAPHENE_REQUIRE_THROW( do_op(edit_license_type_operation(get_license_administrator_id(), lic_id, "x", 0, 10)), fc::exception );

  // Try to set license's eur limit to 0:
  GRAPHENE_REQUIRE_THROW( do_op(edit_license_type_operation(get_license_administrator_id(), lic_id, "x", 200, 0)), fc::exception );

  // Set name to 'standard_plus', amount to 200 and euro limit to 10:
  do_op(edit_license_type_operation(get_license_administrator_id(), lic_id, "standard_plus", 200, 10));

  const auto& lic_plus = _dal.get_license_type("standard_plus");
  BOOST_CHECK(lic_plus.valid());

  BOOST_CHECK_EQUAL( lic_plus->amount.value, 200 );
  BOOST_CHECK_EQUAL( lic_plus->eur_limit.value, 10 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( frequency_record_index_test )
{ try {

  db.create<frequency_history_record_object>([&](frequency_history_record_object& fhro){});
  db.create<frequency_history_record_object>([&](frequency_history_record_object& fhro){});

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_frequency_history_unit_test )
{ try {

  do_op(update_global_frequency_operation(get_license_issuer_id(), 450, "TEST"));
  const auto& fhistory = _dal.get_frequency_history();

  // Here we have object in the history:
  BOOST_CHECK_EQUAL( fhistory.size(), 1 );
  BOOST_CHECK_EQUAL( fhistory[0].frequency.value, 450 );
  BOOST_CHECK_EQUAL( fhistory[0].comment, "TEST" );

  do_op(update_global_frequency_operation(get_license_issuer_id(), 350, "TEST2"));
  const auto& fhistory2 = _dal.get_frequency_history();

  // After second update, there should be two objects in the history:
  BOOST_CHECK_EQUAL( fhistory2.size(), 2 );
  BOOST_CHECK_EQUAL( fhistory2[0].frequency.value, 450 );
  BOOST_CHECK_EQUAL( fhistory2[0].comment, "TEST" );
  BOOST_CHECK_EQUAL( fhistory2[1].frequency.value, 350 );
  BOOST_CHECK_EQUAL( fhistory2[1].comment, "TEST2" );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( get_frequency_history_by_page_unit_test )
{ try {

  for (int i = 0; i < 105; ++i)
    do_op(update_global_frequency_operation(get_license_issuer_id(), 100 + i , "TEST"));

  // This ought to fall, cannot retrieve more than 100 elements:
  GRAPHENE_REQUIRE_THROW( _dal.get_frequency_history_by_page(0, 102), fc::exception );

  // Get first 90 elements:
  const auto& fhistory = _dal.get_frequency_history_by_page(0, 90);
  BOOST_CHECK_EQUAL( fhistory.size(), 90 );
  // 69th element should have frequency set to 169:
  BOOST_CHECK_EQUAL( fhistory[69].frequency.value, 169 );

  // Get 10 elements, starting from 50:
  const auto& fhistory2 = _dal.get_frequency_history_by_page(50, 10);
  BOOST_CHECK_EQUAL( fhistory2.size(), 10 );
  // First element should have frequency set to 150:
  BOOST_CHECK_EQUAL( fhistory2[0].frequency.value, 150 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( locked_license_unit_test )
{ try {
  VAULT_ACTOR(vault);

  auto locked = *(_dal.get_license_type("standard_locked"));
  const share_type bonus_percent = 50;
  share_type frequency_lock = 0;
  const time_point_sec issue_time = db.head_block_time();
  const uint32_t amount = DASCOIN_BASE_STANDARD_CYCLES + (50 * DASCOIN_BASE_STANDARD_CYCLES) / 100;

  // This will fail, frequency cannot be zero:
  GRAPHENE_REQUIRE_THROW( do_op(issue_license_operation(get_license_issuer_id(), vault_id, locked.id,
                                bonus_percent, frequency_lock, issue_time)), fc::exception );

  frequency_lock = 200;
  // Frequency is now valid, this ought to work:
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, locked.id,
                                bonus_percent, frequency_lock, issue_time));

  BOOST_CHECK( vault.license_information.valid() );

  const auto& license_information_obj = (*vault.license_information)(db);

  BOOST_CHECK( license_information_obj.account == vault_id );

  const auto& license_history = license_information_obj.history;

  BOOST_CHECK_EQUAL( license_history.size(), 1 );

  const auto& license_record = license_history[0];

  BOOST_CHECK( license_record.license == locked.id );
  BOOST_CHECK_EQUAL( license_record.amount.value, amount );
  BOOST_CHECK_EQUAL( license_record.base_amount.value, DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( license_record.bonus_percent.value, 50 );
  BOOST_CHECK_EQUAL( license_record.frequency_lock.value, 200 );
  BOOST_CHECK( license_record.activated_at == issue_time );
  BOOST_CHECK( license_record.issued_on_blockchain == issue_time );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( different_license_kinds_unit_test )
{ try {
  VAULT_ACTOR(vault);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto executive_locked = *(_dal.get_license_type("executive_locked"));
  auto standard = *(_dal.get_license_type("standard"));
  const share_type bonus_percent = 50;
  share_type frequency_lock = 20;
  const time_point_sec issue_time = db.head_block_time();

  // Issue standard locked license:
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  // This should work, the same license kind:
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, executive_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  // This should fail, different license kind:
  GRAPHENE_REQUIRE_THROW( do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard.id,
                          bonus_percent, frequency_lock, issue_time)), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_event_index_test )
{ try {

  db.create<upgrade_event_object>([&](upgrade_event_object& lio){});
  db.create<upgrade_event_object>([&](upgrade_event_object& lio){});

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( create_upgrade_event_test )
{ try {

  auto license_administrator_id = db.get_global_properties().authorities.license_administrator;
  auto license_issuer_id = db.get_global_properties().authorities.license_issuer;
  const auto hbt = db.head_block_time();
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  const auto get_upgrade_events = [this](vector<upgrade_event_object> &upgrades) {
    const auto& idx = db.get_index_type<upgrade_event_index>().indices().get<by_id>();
    for ( auto it = idx.begin(); it != idx.end(); ++it )
      upgrades.emplace_back(*it);
  };

  // This should fail, wrong authority:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_issuer_id, hbt, {}, {}, "")), fc::exception );

  // Should also fail, event's execution time is not a multiply of maintenance interval:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_administrator_id,
                                                               dgpo.next_maintenance_time - fc::seconds(1),
                                                               {}, {}, "")), fc::exception );

  // Should also fail, event is scheduled in the past:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_administrator_id,
                                                               dgpo.next_maintenance_time - 2 * gpo.parameters.maintenance_interval,
                                                               {}, {}, "")), fc::exception );

  // Should also fail, subsequent event is scheduled in the past:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_administrator_id,
                                                               dgpo.next_maintenance_time,
                                                               hbt + 60,
                                                               {dgpo.next_maintenance_time - 2 * gpo.parameters.maintenance_interval}, "")), fc::exception );

  // Should also fail, subsequent event is scheduled before the main execution time:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_administrator_id,
                                                               dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                                               hbt + 60,
                                                               {dgpo.next_maintenance_time + gpo.parameters.maintenance_interval}, "")), fc::exception );

  // This ought to work, execute time in future, no cutoff, no subsequent events:
  do_op(create_upgrade_event_operation(license_administrator_id,
                                       dgpo.next_maintenance_time, {}, {}, "foo"));

  vector<upgrade_event_object> upgrades;
  get_upgrade_events(upgrades);

  FC_ASSERT( upgrades.size() == 1 );
  FC_ASSERT( !upgrades[0].executed() );
  FC_ASSERT( upgrades[0].comment.compare("foo") == 0 );
  FC_ASSERT( upgrades[0].execution_time == dgpo.next_maintenance_time );
  FC_ASSERT( *(upgrades[0].cutoff_time) == upgrades[0].execution_time );

  // Fails, cannot create upgrade event at the same time as the previously created event:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_administrator_id, dgpo.next_maintenance_time,
                                                               {}, {}, "")), fc::exception );

  // This ought to work, execute time in future, cutoff in the future, no subsequent events:
  do_op(create_upgrade_event_operation(license_administrator_id,
                                       dgpo.next_maintenance_time + gpo.parameters.maintenance_interval,
                                       hbt + 60, {}, "bar"));

  upgrades.clear();
  get_upgrade_events(upgrades);

  FC_ASSERT( upgrades.size() == 2 );
  FC_ASSERT( !upgrades[1].executed() );
  FC_ASSERT( upgrades[1].comment.compare("bar") == 0 );
  FC_ASSERT( upgrades[1].execution_time == dgpo.next_maintenance_time + gpo.parameters.maintenance_interval );

  // This ought to work, execute time in future, cutoff in the future, subsequent event in the future:
  do_op(create_upgrade_event_operation(license_administrator_id,
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       hbt + 60,
                                       {dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval}, "foobar"));

  upgrades.clear();
  get_upgrade_events(upgrades);

  FC_ASSERT( upgrades.size() == 3 );
  FC_ASSERT( !upgrades[2].executed() );
  FC_ASSERT( upgrades[2].comment.compare("foobar") == 0 );
  FC_ASSERT( upgrades[2].execution_time == dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval );
  FC_ASSERT( upgrades[2].subsequent_execution_times.size() == 1 );
  FC_ASSERT( upgrades[2].subsequent_execution_times[0] == dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval );

  // This fails, second subsequent event is not in the future:
  GRAPHENE_REQUIRE_THROW( do_op(create_upgrade_event_operation(license_administrator_id,
                                                               dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval,
                                                               hbt + 60,
                                                               {dgpo.next_maintenance_time + 4 * gpo.parameters.maintenance_interval,
                                                                dgpo.next_maintenance_time - 4 * gpo.parameters.maintenance_interval}, "")), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_event_executed_test )
{ try {
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  const auto get_upgrade_events = [this](vector<upgrade_event_object> &upgrades) {
    const auto& idx = db.get_index_type<upgrade_event_index>().indices().get<by_id>();
    for ( auto it = idx.begin(); it != idx.end(); ++it )
      upgrades.emplace_back(*it);
  };

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time,
                                       {}, {}, "foo"));

  // Wait for the next maintenance interval:
  generate_blocks(dgpo.next_maintenance_time);

  vector<upgrade_event_object> upgrades;
  get_upgrade_events(upgrades);

  FC_ASSERT( upgrades[0].executed() );

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       {}, {}, "bar"));

  // Wait for the next maintenance interval:
  generate_blocks(dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval);

  upgrades.clear();
  get_upgrade_events(upgrades);

  // At this point, the second upgrade event has been marked as executed:
  FC_ASSERT( upgrades[1].executed() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( update_upgrade_event_test )
{ try {
  auto license_administrator_id = db.get_global_properties().authorities.license_administrator;
  auto license_issuer_id = db.get_global_properties().authorities.license_issuer;
  const auto hbt = db.head_block_time();
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  const auto get_upgrade_events = [this](vector<upgrade_event_object> &upgrades) {
    const auto& idx = db.get_index_type<upgrade_event_index>().indices().get<by_id>();
    for ( auto it = idx.begin(); it != idx.end(); ++it )
      upgrades.emplace_back(*it);
  };

  do_op(create_upgrade_event_operation(license_administrator_id, dgpo.next_maintenance_time, {}, {}, "foo"));

  vector<upgrade_event_object> upgrades;
  get_upgrade_events(upgrades);
  FC_ASSERT( upgrades.size() == 1 );

  // Fails because of invalid authority:
  GRAPHENE_REQUIRE_THROW( do_op(update_upgrade_event_operation(license_issuer_id, upgrades[0].id, hbt, {}, {}, "bar")), fc::exception );

  // Fails because execution time is in the past:
  GRAPHENE_REQUIRE_THROW( do_op(update_upgrade_event_operation(license_administrator_id,
                                                               upgrades[0].id,
                                                               dgpo.next_maintenance_time - 4 * gpo.parameters.maintenance_interval,
                                                               {}, {}, "bar")), fc::exception );

  // Fails because cutoff time is in the past:
  GRAPHENE_REQUIRE_THROW( do_op(update_upgrade_event_operation(license_administrator_id,
                                                               upgrades[0].id,
                                                               dgpo.next_maintenance_time,
                                                               hbt, {}, "bar")), fc::exception );

  // Fails because subsequent execution time is in the past:
  GRAPHENE_REQUIRE_THROW( do_op(update_upgrade_event_operation(license_administrator_id,
                                                               upgrades[0].id,
                                                               dgpo.next_maintenance_time,
                                                               hbt + 60,
                                                               vector<time_point_sec>{dgpo.next_maintenance_time - 4 * gpo.parameters.maintenance_interval}, "bar")), fc::exception );

  // This should succeed:
  do_op(update_upgrade_event_operation(license_administrator_id,
                                       upgrades[0].id,
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       hbt + 120, vector<time_point_sec>{dgpo.next_maintenance_time + 4 * gpo.parameters.maintenance_interval}, "bar"));

  upgrades.clear();
  get_upgrade_events(upgrades);

  // There's still one upgrade event:
  FC_ASSERT( upgrades.size() == 1 );
  // But updated accordingly:
  FC_ASSERT( upgrades[0].execution_time == dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval );
  FC_ASSERT( upgrades[0].cutoff_time == hbt + 120 );
  FC_ASSERT( upgrades[0].subsequent_execution_times.size() == 1 );
  FC_ASSERT( upgrades[0].subsequent_execution_times[0] == dgpo.next_maintenance_time + 4 * gpo.parameters.maintenance_interval );
  FC_ASSERT( upgrades[0].comment.compare("bar") == 0 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( delete_upgrade_event_test )
{ try {
  auto license_administrator_id = db.get_global_properties().authorities.license_administrator;
  auto license_issuer_id = db.get_global_properties().authorities.license_issuer;
  const auto& dgpo = db.get_dynamic_global_properties();

  const auto get_upgrade_events = [this](vector<upgrade_event_object> &upgrades) {
    const auto& idx = db.get_index_type<upgrade_event_index>().indices().get<by_id>();
    for ( auto it = idx.begin(); it != idx.end(); ++it )
      upgrades.emplace_back(*it);
  };

  do_op(create_upgrade_event_operation(license_administrator_id, dgpo.next_maintenance_time, {}, {}, "foo"));

  vector<upgrade_event_object> upgrades;
  get_upgrade_events(upgrades);
  FC_ASSERT( upgrades.size() == 1 );

  // Fails because of invalid authority:
  GRAPHENE_REQUIRE_THROW( do_op(delete_upgrade_event_operation(license_issuer_id, upgrades[0].id)), fc::exception );
  auto id = upgrades[0].id;
  ++id;
  // Fails because of invalid id:
  GRAPHENE_REQUIRE_THROW( do_op(delete_upgrade_event_operation(license_administrator_id, id)), fc::exception );

  do_op(delete_upgrade_event_operation(license_administrator_id, upgrades[0].id));
  upgrades.clear();
  get_upgrade_events(upgrades);

  // No active upgrade events at this point:
  FC_ASSERT( upgrades.empty() );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_cycles_test )
{ try {
  VAULT_ACTOR(foo);
  VAULT_ACTOR(bar);
  VAULT_ACTOR(foobar);
  VAULT_ACTOR(alice);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto executive_locked = *(_dal.get_license_type("executive_locked"));
  auto vice_president_locked = *(_dal.get_license_type("vice_president_locked"));
  auto president_locked = *(_dal.get_license_type("president_locked"));
  const share_type bonus_percent = 0;
  const share_type frequency_lock = 100;
  const time_point_sec issue_time = db.head_block_time();
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  do_op(issue_license_operation(get_license_issuer_id(), foo_id, standard_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, executive_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(issue_license_operation(get_license_issuer_id(), foobar_id, president_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(issue_license_operation(get_license_issuer_id(), alice_id, executive_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(issue_license_operation(get_license_issuer_id(), alice_id, vice_president_locked.id,
                                bonus_percent, frequency_lock, issue_time));

//  generate_blocks(db.head_block_time() + fc::hours(24));

  // No upgrade happened yet!
//  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 0 );

  // Alice is spender:
  do_op(submit_cycles_to_queue_by_license_operation(alice_id, 1000, executive_locked.id, 100, "TEST"));
  do_op(submit_cycles_to_queue_by_license_operation(alice_id, 2000, vice_president_locked.id, 100, "TEST"));

  // FooBar wasted its balance:
  do_op(submit_cycles_to_queue_by_license_operation(foobar_id, DASCOIN_BASE_PRESIDENT_CYCLES, president_locked.id, 100, "TEST"));

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time,
                                       {}, {}, "foo"));

  // Wait for the next maintenance interval:
  generate_blocks(dgpo.next_maintenance_time);

  // One upgrade event has happened:
//  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 1 );

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(bar_id).value, 2 * DASCOIN_BASE_EXECUTIVE_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(foobar_id).value, DASCOIN_BASE_PRESIDENT_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(alice_id).value, 2 * (DASCOIN_BASE_EXECUTIVE_CYCLES - 1000) + 2 * (DASCOIN_BASE_VICE_PRESIDENT_CYCLES - 2000) );

  const auto& license_information_obj = (*alice.license_information)(db);
  const auto& license_history = license_information_obj.history;
  const auto& executive_license_record = license_history[0];
  const auto& vice_president_license_record = license_history[1];
  const uint32_t executive_remaining_cycles = 2 * (DASCOIN_BASE_EXECUTIVE_CYCLES - 1000);
  const uint32_t vice_president_remaining_cycles = 2 * (DASCOIN_BASE_VICE_PRESIDENT_CYCLES - 2000);
  BOOST_CHECK_EQUAL( executive_license_record.amount.value, executive_remaining_cycles );
  BOOST_CHECK_EQUAL( vice_president_license_record.amount.value, vice_president_remaining_cycles );

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       {}, {}, "foo"));

  generate_blocks(dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval);

  // Second upgrade event has happened:
//  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 2 );

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(bar_id).value, 4 * DASCOIN_BASE_EXECUTIVE_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(foobar_id).value, 3 * DASCOIN_BASE_PRESIDENT_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(alice_id).value, 4 * (DASCOIN_BASE_EXECUTIVE_CYCLES - 1000) + 4 * (DASCOIN_BASE_VICE_PRESIDENT_CYCLES - 2000) );

  // Wait for the maintenance interval to trigger:
//  generate_blocks(db.head_block_time() + fc::days(120));
  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval,
                                       {}, {}, "foo"));

  generate_blocks(dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval);

  // Third upgrade event has happened:
//  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 3 );

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(bar_id).value, 4 * DASCOIN_BASE_EXECUTIVE_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(foobar_id).value, 7 * DASCOIN_BASE_PRESIDENT_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(alice_id).value, 4 * (DASCOIN_BASE_EXECUTIVE_CYCLES - 1000) + 4 * (DASCOIN_BASE_VICE_PRESIDENT_CYCLES - 2000) );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_president_cycles_test )
{ try {
  VAULT_ACTOR(foo);
  auto president_locked = *(_dal.get_license_type("president_locked"));
  const share_type bonus_percent = 0;
  const share_type frequency_lock = 100;
  const time_point_sec issue_time = db.head_block_time();
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  do_op(issue_license_operation(get_license_issuer_id(), foo_id, president_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  generate_blocks(db.head_block_time() + fc::hours(24));

  // Submit 1000 cycles, these cannot be upgraded because they are spent:
  do_op(submit_cycles_to_queue_by_license_operation(foo_id, 1000, president_locked.id, 100, "TEST"));

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time,
                                       {}, {}, "foo"));

  // Wait for the next maintenance interval:
  generate_blocks(dgpo.next_maintenance_time);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_PRESIDENT_CYCLES - 1000 );

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       {}, {}, "foo"));

  // Submit 2000 cycles:
  do_op(submit_cycles_to_queue_by_license_operation(foo_id, 2000, president_locked.id, 100, "TEST"));

  // Wait for the next maintenance interval and subsequent upgrade:
  generate_blocks(dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 4 * DASCOIN_BASE_PRESIDENT_CYCLES - 3000 );

  // Submit 3000 cycles:
  do_op(submit_cycles_to_queue_by_license_operation(foo_id, 3000, president_locked.id, 100, "TEST"));

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval,
                                       {}, {}, "foo"));

  // Wait for the next maintenance interval and subsequent upgrade:
  generate_blocks(dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 8 * DASCOIN_BASE_PRESIDENT_CYCLES - 6000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_charter_license_test )
{ try {
  VAULT_ACTOR(foo);
  VAULT_ACTOR(bar);
  VAULT_ACTOR(foobar);
  auto standard_charter = *(_dal.get_license_type("standard_charter"));
  auto executive_charter = *(_dal.get_license_type("executive_charter"));
  auto president_charter = *(_dal.get_license_type("president_charter"));
  const share_type bonus_percent = 0;
  const share_type frequency_lock = 100;
  const time_point_sec issue_time = db.head_block_time();
  const auto &dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  do_op(issue_license_operation(get_license_issuer_id(), foo_id, standard_charter.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(issue_license_operation(get_license_issuer_id(), foobar_id, executive_charter.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(issue_license_operation(get_license_issuer_id(), bar_id, president_charter.id,
                                bonus_percent, frequency_lock, issue_time));

  generate_blocks(db.head_block_time() + fc::hours(24));

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time,
                                       {}, {}, "foo"));

  // Wait for the next maintenance interval:
  generate_blocks(dgpo.next_maintenance_time);

  auto result_vec = *_dal.get_queue_submissions_with_pos(foo_id).result;
  BOOST_CHECK_EQUAL( result_vec.size(), 2 );
  auto rqo = result_vec[1].submission;
  BOOST_CHECK_EQUAL( rqo.origin, "reserve_cycles" );
  BOOST_CHECK_EQUAL( rqo.comment, "Licence Standard Upgrade 1/1" );
  BOOST_CHECK_EQUAL( rqo.amount.value, DASCOIN_BASE_STANDARD_CYCLES );

  auto result_vec2 = *_dal.get_queue_submissions_with_pos(bar_id).result;
  BOOST_CHECK_EQUAL( result_vec2.size(), 2 );
  auto rqo2 = result_vec2[1].submission;
  BOOST_CHECK_EQUAL( rqo2.origin, "reserve_cycles" );
  BOOST_CHECK_EQUAL( rqo2.comment, "Licence President Upgrade 1/3" );
  BOOST_CHECK_EQUAL( rqo2.amount.value, DASCOIN_BASE_PRESIDENT_CYCLES );

  const auto& license_information_obj = (*bar.license_information)(db);

  const auto& license_history = license_information_obj.history;

  BOOST_CHECK_EQUAL( license_history.size(), 1 );

  const auto& license_record = license_history[0];
  auto res = _dal.get_vault_info(bar_id);

  BOOST_CHECK_EQUAL( license_record.amount.value, DASCOIN_BASE_PRESIDENT_CYCLES );

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       {}, {}, "foo"));

  // Wait for the next maintenance interval:
  generate_blocks(dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval);

  auto result_vec3 = *_dal.get_queue_submissions_with_pos(foobar_id).result;
  BOOST_CHECK_EQUAL( result_vec3.size(), 3 );
  auto rqo3 = result_vec3[1].submission;
  BOOST_CHECK_EQUAL( rqo3.origin, "reserve_cycles" );
  BOOST_CHECK_EQUAL( rqo3.comment, "Licence Executive Upgrade 1/2" );
  BOOST_CHECK_EQUAL( rqo3.amount.value, DASCOIN_BASE_EXECUTIVE_CYCLES );
  auto rqo4 = result_vec3[2].submission;
  BOOST_CHECK_EQUAL( rqo4.origin, "reserve_cycles" );
  BOOST_CHECK_EQUAL( rqo4.comment, "Licence Executive Upgrade 2/2" );
  BOOST_CHECK_EQUAL( rqo4.amount.value, 2 * DASCOIN_BASE_EXECUTIVE_CYCLES );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_not_executed_test )
{ try {
  VAULT_ACTOR(foo);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  const share_type bonus_percent = 0;
  const share_type frequency_lock = 100;
  const time_point_sec activated_time = db.head_block_time();
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  do_op(issue_license_operation(get_license_issuer_id(), foo_id, standard_locked.id,
                                bonus_percent, frequency_lock,
                                dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval));

  // This will not upgrade foo, because the execution time is before license's activation time:
  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + gpo.parameters.maintenance_interval, {}, {},
                                       "upgrade1"));

  // Will not upgrade foo, because cutoff time is before license's activation time:
  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval,
                                       activated_time + 60, {}, "upgrade2"));

  // Will not upgrade foo even in subsequent executions:
  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval,
                                       {activated_time + 60},
                                       {{dgpo.next_maintenance_time + 4 * gpo.parameters.maintenance_interval}}, "upgrade3"));

  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, DASCOIN_BASE_STANDARD_CYCLES );

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 4 * gpo.parameters.maintenance_interval,
                                       activated_time - 60, {}, "foo"));

  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, DASCOIN_BASE_STANDARD_CYCLES );

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time + 5 * gpo.parameters.maintenance_interval,
                                       activated_time - 60, {}, "foo"));

  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, DASCOIN_BASE_STANDARD_CYCLES );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_executed_test )
{ try {
  VAULT_ACTOR(foo);
  VAULT_ACTOR(bar);
  VAULT_ACTOR(foobar);
  VAULT_ACTOR(barfoo);
  VAULT_ACTOR(foofoobar);
  VAULT_ACTOR(barbarfoo);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  auto manager_locked = *(_dal.get_license_type("manager_locked"));
  auto pro_locked = *(_dal.get_license_type("pro_locked"));
  auto executive_locked = *(_dal.get_license_type("executive_locked"));
  auto vp_locked = *(_dal.get_license_type("vice_president_locked"));
  auto p_locked = *(_dal.get_license_type("president_locked"));
  const share_type bonus_percent = 0;
  const share_type frequency_lock = 100;
  const time_point_sec activated_time = db.head_block_time();
  const auto& dgpo = db.get_dynamic_global_properties();
  const auto& gpo = db.get_global_properties();

  // License activated before upgrade event, so it should be upgraded:
  do_op(issue_license_operation(get_license_issuer_id(), foo_id, standard_locked.id,
                                bonus_percent, frequency_lock, activated_time));

  // License activated before cutoff time, so it should be upgraded too:
  do_op(issue_license_operation(get_license_issuer_id(), bar_id, manager_locked.id,
                                bonus_percent, frequency_lock, activated_time + fc::hours(26)));

  do_op(create_upgrade_event_operation(get_license_administrator_id(),
                                       dgpo.next_maintenance_time,
                                       activated_time + fc::hours(72),
                                       {{dgpo.next_maintenance_time + 4 * gpo.parameters.maintenance_interval},
                                        {dgpo.next_maintenance_time + 7 * gpo.parameters.maintenance_interval}}, "foo_upgrade"));

  // Issue 200 cycles to foo, those should not be upgraded:
  do_op(issue_cycles_to_license_operation(get_cycle_issuer_id(), foo_id, standard_locked.id, 200, "foo", "bar"));

  generate_blocks(dgpo.next_maintenance_time);

  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(bar_id).value, 2 * DASCOIN_BASE_MANAGER_CYCLES );

  // License created after the first execution of upgrade event, but activated before cutoff time, so it should be upgraded too:
  do_op(issue_license_operation(get_license_issuer_id(), foobar_id, pro_locked.id,
                                bonus_percent, frequency_lock, activated_time + fc::hours(28)));

  generate_blocks(dgpo.next_maintenance_time + 3 * gpo.parameters.maintenance_interval);

  // Balance for the first two should remain the same:
  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(bar_id).value, 2 * DASCOIN_BASE_MANAGER_CYCLES );

  // Upgrade is executed for foobar:
  BOOST_CHECK_EQUAL( get_cycle_balance(foobar_id).value, 2 * DASCOIN_BASE_PRO_CYCLES );

  // License created after cutoff time, but activated before upgrade execution time, so it should be upgraded in encore event:
  do_op(issue_license_operation(get_license_issuer_id(), barfoo_id, executive_locked.id,
                                bonus_percent, frequency_lock, activated_time));

  // License created after cutoff time, but activated before cutoff time, so it should be upgraded in encore event:
  do_op(issue_license_operation(get_license_issuer_id(), foofoobar_id, vp_locked.id,
                                bonus_percent, frequency_lock, activated_time + fc::hours(48)));

  generate_blocks(dgpo.next_maintenance_time + 2 * gpo.parameters.maintenance_interval);

  // Balance for the first three should remain the same:
  BOOST_CHECK_EQUAL( get_cycle_balance(foo_id).value, 2 * DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(bar_id).value, 2 * DASCOIN_BASE_MANAGER_CYCLES );
  BOOST_CHECK_EQUAL( get_cycle_balance(foobar_id).value, 2 * DASCOIN_BASE_PRO_CYCLES );

  // barfoo should be upgraded now:
  BOOST_CHECK_EQUAL( get_cycle_balance(barfoo_id).value, 2 * DASCOIN_BASE_EXECUTIVE_CYCLES );
  // foofoobar also:
  BOOST_CHECK_EQUAL( get_cycle_balance(foofoobar_id).value, 2 * DASCOIN_BASE_VICE_PRESIDENT_CYCLES );

  // License activated after cutoff time, should not be upgraded even in encore:
  do_op(issue_license_operation(get_license_issuer_id(), barbarfoo_id, p_locked.id,
                                bonus_percent, frequency_lock, activated_time + fc::hours(78)));
  generate_blocks(db.head_block_time() + fc::hours(25));

  // Cycles remain:
  BOOST_CHECK_EQUAL( get_cycle_balance(barbarfoo_id).value, DASCOIN_BASE_PRESIDENT_CYCLES );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( update_license_unit_test )
{ try {
  VAULT_ACTOR(vault);

  auto standard_locked = *(_dal.get_license_type("standard_locked"));
  const share_type bonus_percent = 50;
  share_type frequency_lock = 20;
  const time_point_sec issue_time = db.head_block_time();
  const uint32_t amount_after = DASCOIN_BASE_STANDARD_CYCLES + (55 * DASCOIN_BASE_STANDARD_CYCLES) / 100;

  // Issue standard locked license:
  do_op(issue_license_operation(get_license_issuer_id(), vault_id, standard_locked.id,
                                bonus_percent, frequency_lock, issue_time));

  do_op(update_license_operation(get_license_issuer_id(), vault_id, standard_locked.id, 55, 10, issue_time + 3600));

  const auto& license_information_obj = (*vault.license_information)(db);

  BOOST_CHECK( license_information_obj.account == vault_id );

  const auto& license_history = license_information_obj.history;

  BOOST_CHECK_EQUAL( license_history.size(), 1 );

  const auto& license_record = license_history[0];
  auto res = _dal.get_vault_info(vault_id);

  BOOST_CHECK( license_record.license == standard_locked.id );
  BOOST_CHECK_EQUAL( license_record.amount.value, amount_after );
  BOOST_CHECK_EQUAL( license_record.base_amount.value, DASCOIN_BASE_STANDARD_CYCLES );
  BOOST_CHECK_EQUAL( license_record.bonus_percent.value, 55 );
  BOOST_CHECK_EQUAL( license_record.frequency_lock.value, 10 );
  BOOST_CHECK( license_record.activated_at == issue_time + 3600 );
  BOOST_CHECK_EQUAL( res->free_cycle_balance.value, amount_after );

  auto executive_locked = *(_dal.get_license_type("executive_locked"));

  // This ought to fail, this license has not been issued to the vault:
  GRAPHENE_REQUIRE_THROW( do_op(update_license_operation(get_license_issuer_id(), vault_id, executive_locked.id, 40, 10, issue_time + 3600)), fc::exception );

  // This ought to fail, frequency lock cannot be zero:
  GRAPHENE_REQUIRE_THROW( do_op(update_license_operation(get_license_issuer_id(), vault_id, standard_locked.id, 40, 0, issue_time + 3600)), fc::exception );

  // This ought to fail, bonus percentage cannot be decreased:
  GRAPHENE_REQUIRE_THROW( do_op(update_license_operation(get_license_issuer_id(), vault_id, standard_locked.id, 30, 10, issue_time + 3600)), fc::exception );

  // Now submit 1000 cycles:
  do_op(submit_cycles_to_queue_by_license_operation(vault_id, 1000, standard_locked.id, 10, "TEST"));

  do_op(update_license_operation(get_license_issuer_id(), vault_id, standard_locked.id, 60, 20, {}));

  auto res2 = _dal.get_vault_info(vault_id);
  // Now we should have 55 cycles extra, but 1000 is spent:
  BOOST_CHECK_EQUAL( res2->free_cycle_balance.value, amount_after + 55 - 1000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

/*BOOST_FIXTURE_TEST_SUITE( dascoin_tests, database_fixture )

BOOST_FIXTURE_TEST_SUITE( license_tests, database_fixture )

BOOST_AUTO_TEST_CASE( issue_single_license_test )
{ try {
  VAULT_ACTOR(vault);
  const auto pro_id = get_license_type("pro").id;

  issue_license_to_vault_account(vault_id, pro_id, 0, 0);
  generate_blocks_until_license_approved();

  BOOST_CHECK( vault.license_info.max_license() == pro_id );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_license_with_bonus_cycles )
{ try {
  const frequency_type frequency_lock = 200;
  VAULT_ACTOR(v100);
  VAULT_ACTOR(v150);
  VAULT_ACTOR(v50);
  VAULT_ACTOR(vzero);
  VAULT_ACTOR(vneg)
  vector<license_request_object> requests;

  const auto& get_pending_request = [&](const account_object& account) -> const license_request_object&
  {
    return (*account.license_info.pending).request(db);
  };
  
  issue_license_to_vault_account(v100, "standard_charter", 0, frequency_lock);
  issue_license_to_vault_account(v150, "standard_charter", 50, frequency_lock);
  issue_license_to_vault_account(v50, "standard_charter", -50, frequency_lock);
  GRAPHENE_CHECK_THROW( issue_license_to_vault_account(vzero, "standard_charter", -100, frequency_lock), fc::exception );
  GRAPHENE_CHECK_THROW( issue_license_to_vault_account(vneg, "standard_charter", -200, frequency_lock), fc::exception );

  BOOST_CHECK_EQUAL( get_pending_request(v100).amount.value, 100 );
  BOOST_CHECK_EQUAL( get_pending_request(v150).amount.value, 150 );
  BOOST_CHECK_EQUAL( get_pending_request(v50).amount.value, 50 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( license_type_integrity_test )
{ try {
  // TODO: test for every type of license there is.

  auto lic_obj = get_license_type("standard");

  BOOST_CHECK_EQUAL( lic_obj.name, "standard" );
  BOOST_CHECK_EQUAL( lic_obj.amount.value, 100 );
  BOOST_CHECK_EQUAL( lic_obj.kind, license_kind::regular );
  BOOST_CHECK( lic_obj.balance_upgrade == upgrade_type({2}) );
  BOOST_CHECK( lic_obj.requeue_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.return_upgrade == upgrade_type() );

  lic_obj = get_license_type("standard_charter");

  BOOST_CHECK_EQUAL( lic_obj.name, "standard_charter" );
  BOOST_CHECK_EQUAL( lic_obj.amount.value, 100 );
  BOOST_CHECK_EQUAL( lic_obj.kind, license_kind::chartered );
  BOOST_CHECK( lic_obj.balance_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.requeue_upgrade == upgrade_type({1}) );
  BOOST_CHECK( lic_obj.return_upgrade == upgrade_type() );

  lic_obj = get_license_type("standard-promo");

  BOOST_CHECK_EQUAL( lic_obj.name, "standard-promo" );
  BOOST_CHECK_EQUAL( lic_obj.amount.value, 100 );
  BOOST_CHECK_EQUAL( lic_obj.kind, license_kind::promo );
  BOOST_CHECK( lic_obj.balance_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.requeue_upgrade == upgrade_type() );
  BOOST_CHECK( lic_obj.return_upgrade == upgrade_type({1}) );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( check_issue_frequency_lock_not_zero )
{ try {
  VAULT_ACTOR(vault);

  // Regular license can have a frequency lock of 0:
  issue_license_to_vault_account(vault_id, get_license_type("pro").id, 0, 0);

  // Charter license CANNOT have a frequency lock of 0:
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(vault_id, get_license_type("pro_charter").id, 0, 0), fc::exception );

  // Promo license CANNOT have a frequency lock of 0:
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(vault_id, get_license_type("pro-promo").id, 0, 0), fc::exception );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_type_test )
{ try {

  upgrade_type test_pres_charter_upgrade({1,2,2});
  share_type x = 1000;

  // 1000 x1 = 1000
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 1000 );

  // 1000 x2 = 2000
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 2000 );

  // 2000 x2 = 4000
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 4000 );

  // After this it stays the same:
  x = test_pres_charter_upgrade(x);
  BOOST_CHECK_EQUAL( x.value, 4000 );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( issue_license_test )
{ try {
  ACTOR(wallet);
  VAULT_ACTOR(stan);
  VAULT_ACTOR(allguy);

  const auto& check_pending = [&](const account_object& acc, const string& lic_name)
  {
    auto lic = get_license_type(lic_name);
    auto pending = *(acc.license_info.pending);
    BOOST_CHECK( lic.id == pending.license );
  };

  // Rejected: cannot issue to a vault account.
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(wallet, "standard"), fc::exception );

  // Issue standard license to our old pal Stan, and Allguy:
  issue_license_to_vault_account(stan, "standard");
  check_pending(stan, "standard");

  issue_license_to_vault_account(allguy, "standard");
  check_pending(allguy, "standard");

  // Try and issue another license to stan:
  GRAPHENE_REQUIRE_THROW( issue_license_to_vault_account(stan, "manager_charter", 0, 200), fc::exception );

  generate_blocks_until_license_approved();

  // Check if Stan has 100 cycles:
  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 100 );

  // Now we try the Allguy:
  // Allguy should get all the licenses in order:
  license_information lic_info;
  issue_license_to_vault_account(allguy, "manager");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2}) );

  issue_license_to_vault_account(allguy, "pro");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 2600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2}) );

  issue_license_to_vault_account(allguy, "executive");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 7600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2,2}) );

  issue_license_to_vault_account(allguy, "president");
  generate_blocks_until_license_approved();
  BOOST_CHECK_EQUAL( get_cycle_balance(allguy_id).value, 32600 );
  lic_info = allguy_id(db).license_info;
  BOOST_CHECK( lic_info.balance_upgrade == upgrade_type({2,2,2}) );

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE( upgrade_cycles_test )
{ try {
  VAULT_ACTOR(stan);
  generate_block();
  VAULT_ACTOR(richguy);
  generate_block();
  ACTOR(wallet);
  generate_block();

  issue_license_to_vault_account(stan, "standard");  // 100 cycles.
  issue_license_to_vault_account(richguy, "president");  // 25000 cycles.

  adjust_cycles(wallet_id, 2000);  // Wallet has no license, but has floating cycles.

  // Wait for time to elapse:
  // TODO: fetch the time parameter.
  generate_blocks(db.head_block_time() + fc::hours(24));

  // Check balances on accounts:
  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 100 );  // 100
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 25000 );  // 25000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // 2000 (no license)

  // No upgrade happened yet!
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 0 );

  // Wait for the maintenace interval to trigger:
  generate_blocks(db.head_block_time() + fc::days(120));
  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);  // Just in case, on the maintenance int.

  // One upgrade event has happened:
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 1 );

  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 200 );  // 100 -> 200
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 50000 );  // 25000 -> 50000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // Wallet should get no increase.

  // Wait for the maintenace interval to trigger:
  generate_blocks(db.head_block_time() + fc::days(120));
  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  // Second upgrade event has happened:
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 2 );

  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 200 );
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 100000 );  // 50000 -> 100000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // No increase.

  // Wait for the maintenace interval to trigger:
  generate_blocks(db.head_block_time() + fc::days(120));
  generate_blocks(db.get_dynamic_global_properties().next_maintenance_time);

  // Second upgrade event has happened:
  BOOST_CHECK_EQUAL( db.get_dynamic_global_properties().total_upgrade_events, 3 );

  BOOST_CHECK_EQUAL( get_cycle_balance(stan_id).value, 200 );
  BOOST_CHECK_EQUAL( get_cycle_balance(richguy_id).value, 200000 );  //  100000 -> 200000
  BOOST_CHECK_EQUAL( get_cycle_balance(wallet_id).value, 2000 );  // No increase.

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()*/
