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

    struct update_global_parameters_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;

      account_id_type   authority;
      chain_parameters  new_parameters;

      extensions_type extensions;

      update_global_parameters_operation() = default;
      explicit update_global_parameters_operation(
              const account_id_type& authority,
              const chain_parameters& new_parameters)
              : authority(authority),
                new_parameters(new_parameters) {}

      account_id_type fee_payer() const { return authority; }
      share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
      void validate() const {}
    };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::update_global_parameters_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::update_global_parameters_operation,
            (fee)
            (authority)
            (new_parameters)
            (extensions)
          )
