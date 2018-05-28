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

#include "database_fixture.hpp"
#include <graphene/chain/daspay_object.hpp>


using namespace graphene::chain::test;

namespace graphene { namespace chain {

  void database_fixture::set_daspay_transaction_ratio(account_id_type authority, share_type debit_ratio, share_type credit_ratio)
  {
    set_daspay_transaction_ratio_operation op;
    op.authority = authority;
    op.debit_ratio = debit_ratio;
    op.credit_ratio = credit_ratio;
    signed_transaction tx;
    set_expiration(db, tx);
    tx.operations.push_back(op);
    tx.validate();
    processed_transaction ptx = db.push_transaction(tx, ~0);
    tx.clear();
  }

} }  // namespace graphene::chain
