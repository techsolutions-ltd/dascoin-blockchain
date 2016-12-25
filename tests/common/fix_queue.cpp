/**
 * DASCOIN!
 */
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include <graphene/account_history/account_history_plugin.hpp>
#include <graphene/market_history/market_history_plugin.hpp>

#include <graphene/db/simple_index.hpp>

#include <graphene/chain/protocol/cycle.hpp>

#include <graphene/utilities/tempdir.hpp>

#include <fc/crypto/digest.hpp>
#include <fc/smart_ref_impl.hpp>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "database_fixture.hpp"

using namespace graphene::chain::test;

namespace graphene { namespace chain {

  void database_fixture::submit_cycles(account_id_type account_id, share_type amount)
  { try {

    submit_cycles_operation op;
    op.account = account_id;
    op.amount = amount;

    set_expiration(db, trx);
    trx.operations.clear();
    trx.operations.push_back(op);
    trx.validate();
    db.push_transaction(trx, ~0);

  } FC_LOG_AND_RETHROW() };

} }  // graphene::chain
