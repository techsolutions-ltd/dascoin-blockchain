/*
 * MIT License
 *
 * Copyright (c) 2018 TechSolutions Ltd.
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
