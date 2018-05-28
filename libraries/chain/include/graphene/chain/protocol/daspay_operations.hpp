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

    struct set_daspay_transaction_ratio_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;
      account_id_type authority;
      share_type debit_ratio;
      share_type credit_ratio;
      extensions_type extensions;

      set_daspay_transaction_ratio_operation() = default;
      explicit set_daspay_transaction_ratio_operation(
              const account_id_type& authority,
              const share_type& debit_ratio,
              const share_type& credit_ratio)
        : authority(authority),
          debit_ratio(debit_ratio),
          credit_ratio(credit_ratio) {}

      account_id_type fee_payer() const { return authority; }
      share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
      void validate() const;
    };

    struct register_daspay_authority_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;

      account_id_type issuer;
      public_key_type daspay_public_key;

      extensions_type extensions;

      register_daspay_authority_operation() = default;
      explicit register_daspay_authority_operation(account_id_type issuer, public_key_type auth)
              : issuer(issuer)
              , daspay_public_key(auth) {}

      account_id_type fee_payer() const { return issuer; }
      void validate() const {};
      share_type calculate_fee(const fee_parameters_type&) const { return 0; }
    };

    struct daspay_debit_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;

      account_id_type issuer;
      share_type amount;
      public_key_type auth_key;

      extensions_type extensions;

      daspay_debit_operation() = default;
      explicit daspay_debit_operation(account_id_type issuer, share_type amount, public_key_type auth_key)
              : issuer(issuer)
              , amount(amount)
              , auth_key(auth_key) {}

      account_id_type fee_payer() const { return GRAPHENE_TEMP_ACCOUNT; }
      void validate() const {};
      share_type calculate_fee(const fee_parameters_type&) const { return 0; }
      void get_required_authorities( vector<authority>& o ) const
      {
        authority auth;
        auth.key_auths[auth_key] = 1;
        auth.weight_threshold = 1;
        o.emplace_back( std::move(auth) );
      }
    };

  } }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::set_daspay_transaction_ratio_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::set_daspay_transaction_ratio_operation,
            (fee)
            (authority)
            (debit_ratio)
            (credit_ratio)
            (extensions)
          )

FC_REFLECT( graphene::chain::register_daspay_authority_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::register_daspay_authority_operation,
            (fee)
            (issuer)
            (daspay_public_key)
            (extensions)
          )

FC_REFLECT( graphene::chain::daspay_debit_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::daspay_debit_operation,
            (fee)
            (issuer)
            (amount)
            (auth_key)
            (extensions)
          )
