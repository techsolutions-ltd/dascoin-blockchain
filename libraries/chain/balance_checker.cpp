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

#include <graphene/chain/balance_checker.hpp>
#include <graphene/chain/hardfork.hpp>

namespace graphene
{
  namespace chain
  {
    void_result balance_checker::check_remaining_balance(const database& db, const account_object& account,
                                                         const asset_object& asset, const graphene::chain::asset& amount_to_spend)
    {
      try
      {
        if (db.head_block_time() > HARDFORK_BLC_216_TIME)
          return {};

        if (db.head_block_time() > HARDFORK_EXEX_127_TIME)
        {
          if (asset.id == db.get_dascoin_asset_id() && !account.is_custodian())
          {
            int minimum_dsc_no_precision = MINIMUM_DSC_BALANCE / DASCOIN_DEFAULT_ASSET_PRECISION;
            FC_ASSERT( db.get_balance(account, asset).amount - amount_to_spend.amount >= MINIMUM_DSC_BALANCE,
		       "insufficient balance, there must be ${a} dsc remaining", ("a", minimum_dsc_no_precision));
          }
        }

        return {};
      }
      FC_CAPTURE_AND_RETHROW()
    }
  }
}
