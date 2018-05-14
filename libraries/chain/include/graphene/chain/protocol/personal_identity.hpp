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

#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

///////////////////////////////
// Operations:               //
///////////////////////////////

  struct update_pi_limits_operation : public base_operation
  {
    struct fee_parameters_type {};

    asset fee;
    account_id_type pi_validator;  // This is the id of the validation authority that assigns the pi and levels.

    account_id_type account;  // The account whose level and limits are being modified.
    uint8_t level;  // The new level of PI.
    optional<limits_type> new_limits;  // (optional) New limits.

    extensions_type extensions;

    account_id_type fee_payer()const { return pi_validator; }
    void validate()const;
    share_type calculate_fee(const fee_parameters_type& k)const { return 0; }
  };

} }  // namespace graphene::chain

////////////////////////////////
// Reflections:               //
////////////////////////////////

FC_REFLECT( graphene::chain::update_pi_limits_operation::fee_parameters_type,  )

FC_REFLECT( graphene::chain::update_pi_limits_operation,
            (fee)
            (pi_validator)
            (account)
            (level)
            (new_limits)
            (extensions)
          )
