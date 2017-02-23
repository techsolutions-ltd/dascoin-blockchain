/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/protocol/cycle.hpp>
#include <graphene/chain/queue_objects.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

  void database_fixture::adjust_frequency(frequency_type f)
  {
    db.modify(get_dynamic_global_properties(), [f](dynamic_global_property_object& dgpo){
      dgpo.frequency = f;
    });
  }

  void database_fixture::adjust_dascoin_reward(uint32_t amount)
  {
    db.modify(get_global_properties(), [amount](global_property_object& gpo){
      gpo.parameters.dascoin_reward_amount = amount;
    });
  }

  void database_fixture::submit_cycles(account_id_type account_id, share_type amount)
  { try {

    submit_cycles_to_queue_operation op;
    op.account = account_id;
    op.amount = amount;

    set_expiration(db, trx);
    trx.operations.clear();
    trx.operations.push_back(op);
    trx.validate();
    db.push_transaction(trx, ~0);

  } FC_LOG_AND_RETHROW() };

  vector<reward_queue_object> database_fixture::get_reward_queue_objects_by_time()
  { try {

    vector<reward_queue_object> result;
    const auto& idx = db.get_index_type<reward_queue_index>().indices().get<by_time>();
    for ( auto req: idx )
      result.emplace_back(req);
    return result;

  } FC_LOG_AND_RETHROW() };

  vector<reward_queue_object> database_fixture::get_reward_queue_objects_by_account(account_id_type account_id)
  { try {

    vector<reward_queue_object> result;
    const auto& idx = db.get_index_type<reward_queue_index>().indices().get<by_account>();
    for( auto itr = idx.find(boost::make_tuple(account_id)); itr != idx.end() && itr->account == account_id; itr++)
      result.emplace_back(*itr);
    return result;

  } FC_LOG_AND_RETHROW() };

  void database_fixture::toggle_reward_queue(bool state)
  { try {

    db.modify(get_global_properties(), [state](global_property_object& gpo){
      gpo.parameters.enable_dascoin_queue = state;
    });

  } FC_LOG_AND_RETHROW() };

  uint32_t database_fixture::get_reward_queue_size() const
  {
    return db.get_index_type<reward_queue_index>().indices().size();
  }

} }  // graphene::chain
