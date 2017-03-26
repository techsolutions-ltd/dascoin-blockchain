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

  void database_fixture::adjust_dascoin_reward(share_type amount)
  {
    db.modify(get_global_properties(), [amount](global_property_object& gpo){
      gpo.parameters.dascoin_reward_amount = amount;
    });
  }

  void database_fixture::toggle_reward_queue(bool state)
  { try {

    db.modify(get_global_properties(), [state](global_property_object& gpo){
      gpo.parameters.enable_dascoin_queue = state;
    });

  } FC_LOG_AND_RETHROW() };

} }  // graphene::chain
