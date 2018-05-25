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

#include <graphene/chain/protocol/daspay_operations.hpp>

// TODO: set_daspay_ratio()
// TODO: CRUD payment_service_provider map
// TODO: blacklist operation
// TODO: daspay_debit_transaction()
// TODO: daspay_credit_transaction()
// TODO: register_daspay_authority()
// TODO: reserve_asset()
// TODO: unreserve_asset()

namespace graphene { namespace chain {

    void set_daspay_transaction_ratio_operation::validate() const
    {
      FC_ASSERT( debit_ratio >= 0 && debit_ratio < 10000 );
      FC_ASSERT( credit_ratio >= 0 && credit_ratio < 10000 );
    }

  } } // namespace graphene::chain
