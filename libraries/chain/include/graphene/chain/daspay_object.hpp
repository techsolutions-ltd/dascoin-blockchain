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
#include <graphene/chain/protocol/types.hpp>
#include <graphene/db/generic_index.hpp>
#include <graphene/db/object.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace graphene { namespace chain {

  ///////////////////////////////
  // OBJECTS:                  //
  ///////////////////////////////

  class payment_service_provider_object : public graphene::db::abstract_object<payment_service_provider_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_payment_service_provider_object_type;

    account_id_type payment_service_provider_account;
    vector<account_id_type> payment_service_provider_clearing_accounts;
    extensions_type extensions;

    void validate() const;
  };

  /**
   * @class daspay_authority_object
   * @brief DasPay authority object keeps info on opted in payment provider per user
   * @ingroup object
   *
   */

  class daspay_authority_object : public abstract_object<daspay_authority_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_daspay_authority_object_type;

    account_id_type daspay_user;
    account_id_type payment_provider;
    public_key_type daspay_public_key;
    optional<string> memo;

    extensions_type extensions;

    daspay_authority_object() = default;
    explicit daspay_authority_object(account_id_type daspay_user, account_id_type payment_provider,
                                     public_key_type daspay_public_key, optional<string> memo)
            : daspay_user(daspay_user),
              payment_provider(payment_provider),
              daspay_public_key(std::move(daspay_public_key)),
              memo(memo)
    {}
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////
  struct by_payment_service_provider;
  typedef multi_index_container<
    payment_service_provider_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member<object, object_id_type, &object::id>
      >,
      ordered_unique<
        tag<by_payment_service_provider>,
        member<payment_service_provider_object, account_id_type, &payment_service_provider_object::payment_service_provider_account>
      >
    >
  > payment_service_provider_multi_index_type;

  typedef generic_index<payment_service_provider_object, payment_service_provider_multi_index_type> payment_service_provider_index;

  struct by_daspay_user;
  struct by_payment_provider;
  using daspay_authority_multi_index_type = multi_index_container<
    daspay_authority_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique<
        tag<by_daspay_user>,
          composite_key< daspay_authority_object,
            member< daspay_authority_object, account_id_type, &daspay_authority_object::daspay_user >,
            member< object, object_id_type, &object::id >
          >
      >,
      ordered_unique<
        tag<by_payment_provider>,
          composite_key< daspay_authority_object,
            member< daspay_authority_object, account_id_type, &daspay_authority_object::payment_provider >,
            member< daspay_authority_object, account_id_type, &daspay_authority_object::daspay_user >
          >
      >
    >
  >;

  using daspay_authority_index = generic_index<daspay_authority_object, daspay_authority_multi_index_type>;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::payment_service_provider_object, (graphene::db::object),
                    (payment_service_provider_account)
                    (payment_service_provider_clearing_accounts)
                    (extensions)
                  )

FC_REFLECT_DERIVED( graphene::chain::daspay_authority_object, (graphene::db::object),
                    (daspay_user)
                    (payment_provider)
                    (daspay_public_key)
                    (memo)
                  )
