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

#include <graphene/db/object.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/chain/protocol/base.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <boost/multi_index/composite_key.hpp>




namespace graphene { namespace chain {

    class payment_provider_object : public graphene::db::abstract_object<payment_provider_object>
    {
    public:
      static const uint8_t space_id = implementation_ids;
      static const uint8_t type_id  = impl_payment_provider_object_type;

      account_id_type payment_provider_account;
      vector<account_id_type> payment_provider_clearing_accounts;
      extensions_type extensions;

      void validate() const;
    };

///////////////////////////////
// MULTI INDEX CONTAINERS:   //
///////////////////////////////
    struct by_provider;
    typedef multi_index_container<
            payment_provider_object,
            indexed_by<
                    ordered_unique<
                            tag<by_id>, member<object, object_id_type, &object::id>
                    >,
                    ordered_unique< tag<by_provider>,
                            member<payment_provider_object, account_id_type, &payment_provider_object::payment_provider_account>
                    >
            >
    > payment_provider_multi_index_type;

    typedef generic_index<payment_provider_object, payment_provider_multi_index_type> payment_provider_index;

  } }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::payment_provider_object, (graphene::db::object),
                    (payment_provider_account)
                    (payment_provider_clearing_accounts)
                    (extensions)
)
