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

#include <graphene/chain/protocol/das33_operations.hpp>

// TODO: blacklist operation

namespace graphene { namespace chain {

  void das33_project_create_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    const size_t len = name.size();
    FC_ASSERT ( len >= GRAPHENE_MIN_ACCOUNT_NAME_LENGTH );
    FC_ASSERT ( len <= GRAPHENE_MAX_ACCOUNT_NAME_LENGTH );
    FC_ASSERT ( goal_amount_eur > 0);
  }

  void das33_project_update_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    if(name)
    {
      const size_t len = (*name).size();
      FC_ASSERT ( len >= GRAPHENE_MIN_ACCOUNT_NAME_LENGTH );
      FC_ASSERT ( len <= GRAPHENE_MAX_ACCOUNT_NAME_LENGTH );
    }
    if (goal_amount)
    {
      FC_ASSERT ( *goal_amount > 0 );
    }
  }

  void das33_project_delete_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
  }

  void das33_pledge_asset_operation::validate() const
  {
    FC_ASSERT( fee.amount >= 0 );
    FC_ASSERT( pledged.amount > 0, "Must submit a non-zero value" );
    FC_ASSERT( account_id != account_id_type(), "Illegal account id");

    // For now, only dascoins can be pledged
    // TODO: add bitcoin
    FC_ASSERT( pledged.asset_id == asset_id_type{DASCOIN_DASCOIN_INDEX}, "Illegal asset pledged");
  }

  void das33_project_complete_operation::validate() const
  {  }

  void das33_project_reject_operation::validate() const
  {  }

  void das33_pledge_result_operation::validate() const
  {  }

} } // namespace graphene::chain
