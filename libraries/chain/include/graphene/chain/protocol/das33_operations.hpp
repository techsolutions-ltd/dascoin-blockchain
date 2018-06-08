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

    struct das33_pledge_cycles_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;

      account_id_type        vault_id;
      license_type_id_type   license_id;
      share_type             cycles_amount;
      das33_project_id_type  project_id;
      extensions_type        extensions;

      das33_pledge_cycles_operation() = default;

      explicit das33_pledge_cycles_operation(const account_id_type& vault_id,
                                             const license_type_id_type& license_id,
                                             share_type cycles_amount,
                                             const das33_project_id_type& project_id)
              : vault_id(vault_id)
              , license_id(license_id)
              , cycles_amount(cycles_amount)
              , project_id(project_id) {}

      account_id_type fee_payer() const { return vault_id; }
      void validate() const;
      share_type calculate_fee(const fee_parameters_type&) const { return 0; }
    };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::das33_pledge_cycles_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::das33_pledge_cycles_operation,
            (fee)
            (vault_id)
            (license_id)
            (cycles_amount)
            (project_id)
            (extensions)
          )
