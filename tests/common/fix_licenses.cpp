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

#include <graphene/chain/license_objects.hpp>
// #include <graphene/chain/account_object.hpp>
// #include <graphene/chain/asset_object.hpp>
// #include <graphene/chain/committee_member_object.hpp>
// #include <graphene/chain/fba_object.hpp>
// #include <graphene/chain/market_object.hpp>
// #include <graphene/chain/vesting_balance_object.hpp>
// #include <graphene/chain/witness_object.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

const license_type_object& database_fixture::create_license_type(const string& kind, const string& name,
                                                                 share_type amount, 
                                                                 upgrade_multiplier_type balance_multipliers,
                                                                 upgrade_multiplier_type requeue_multipliers,
                                                                 upgrade_multiplier_type return_multipliers,
                                                                 share_type eur_limit)
{ try {
  create_license_type_operation op;
  op.admin = get_license_administrator_id();
  op.name = name;
  op.amount = amount;
  op.kind = kind;
  op.balance_multipliers = balance_multipliers;
  op.requeue_multipliers = requeue_multipliers;
  op.return_multipliers = return_multipliers;

  set_expiration(db, trx);
  trx.operations.clear();
  trx.operations.push_back(op);
  trx.validate();
  processed_transaction ptx = db.push_transaction(trx, ~0);
  trx.operations.clear();

  return db.get<license_type_object>( ptx.operation_results[0].get<object_id_type>() );

} FC_LOG_AND_RETHROW() }

} }  // namespace graphene::chain
