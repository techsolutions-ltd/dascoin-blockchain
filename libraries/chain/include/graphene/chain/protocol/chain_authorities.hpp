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

#include <graphene/chain/protocol/types.hpp>

namespace graphene { namespace chain {

  using namespace graphene::chain::util;

  struct chain_authorities : public linkable_struct<account_id_type>
  {
    account_id_type root_administrator;
    // License related:
    account_id_type license_administrator;
    account_id_type license_issuer;
    account_id_type license_authenticator;
    // Webasset related:
    account_id_type webasset_issuer;
    account_id_type webasset_authenticator;
    // Cycle related:
    account_id_type cycle_issuer;
    account_id_type cycle_authenticator;
    // Registration of accounts:
    account_id_type registrar;
    // Validation of personal information:
    account_id_type pi_validator;
    // Handling of wire_out payments:
    account_id_type wire_out_handler;
    // Daspay:
    account_id_type daspay_administrator;

    LINK_ENUM_TO_FIELDS( chain_authority_kind,
                         (root_administrator)
                         (license_administrator)
                         (license_issuer)
                         (license_authenticator)
                         (webasset_issuer)
                         (webasset_authenticator)
                         (cycle_issuer)
                         (cycle_authenticator)
                         (registrar)
                         (pi_validator)
                         (wire_out_handler)
			 (daspay_administrator)
                       );

  };

} } // namespace graphene::chain

FC_REFLECT( graphene::chain::chain_authorities,
            (root_administrator)
            (license_administrator)
            (license_issuer)
            (license_authenticator)
            (webasset_issuer)
            (webasset_authenticator)
            (cycle_issuer)
            (cycle_authenticator)
            (registrar)
            (pi_validator)
            (wire_out_handler)
	    (daspay_administrator)
          );
