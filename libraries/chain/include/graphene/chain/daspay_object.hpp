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

#include <graphene/chain/database.hpp>
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

  class delayed_operation_object : public abstract_object<delayed_operation_object>
  {
  public:
    static const uint8_t space_id = implementation_ids;
    static const uint8_t type_id  = impl_delayed_operation_object_type;

    account_id_type account;
    operation op;
    fc::time_point_sec issued_time;
    uint32_t skip;

    extensions_type extensions;

    int which() const {
      return op.which();
    }

    delayed_operation_object() = default;
    explicit delayed_operation_object(account_id_type account,
                                             operation op,
                                             fc::time_point_sec issued_time,
                                             uint32_t skip)
            : account(account),
              op(op),
              issued_time(issued_time),
              skip(skip)
    {}
  };

  struct op_visitor
  {
    using result_type = void;

    database& _db;

    op_visitor(database &d)
            : _db(d) {}

    template<typename T>
    result_type operator()( const T& v ) const
    {
      FC_ASSERT( false, "Handler for delayed operation not implemented" );
    }

    result_type operator()( const unreserve_asset_on_account_operation& op ) const
    {
      ilog("unreserving ${d}", ("d", _db.to_pretty_string(op.asset_to_unreserve)));
      _db.adjust_balance( op.account, asset{ op.asset_to_unreserve.amount, _db.get_dascoin_asset_id() }, -op.asset_to_unreserve.amount );
      _db.push_applied_operation(unreserve_completed_operation{op.account, op.asset_to_unreserve});
    }
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////
  struct by_payment_service_provider;
  using payment_service_provider_multi_index_type = multi_index_container<
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
  >;

  using payment_service_provider_index = generic_index<payment_service_provider_object, payment_service_provider_multi_index_type>;

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
            member< daspay_authority_object, account_id_type, &daspay_authority_object::daspay_user >,
            member< daspay_authority_object, account_id_type, &daspay_authority_object::payment_provider >
        >
      >
    >
  >;

  using daspay_authority_index = generic_index<daspay_authority_object, daspay_authority_multi_index_type>;

  struct by_account;
  struct by_operation;
  using delayed_operations_multi_index_type = multi_index_container<
    delayed_operation_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_unique<
        tag<by_account>,
          composite_key< delayed_operation_object,
            member< delayed_operation_object, account_id_type, &delayed_operation_object::account >,
            member< object, object_id_type, &object::id >
          >
      >,
      ordered_unique<
        tag<by_operation>,
          composite_key< delayed_operation_object,
            member< delayed_operation_object, account_id_type, &delayed_operation_object::account >,
            const_mem_fun< delayed_operation_object, int, &delayed_operation_object::which >
          >
      >
    >
  >;

  using delayed_operations_index = generic_index<delayed_operation_object, delayed_operations_multi_index_type>;

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

FC_REFLECT_DERIVED( graphene::chain::delayed_operation_object, (graphene::db::object),
                    (account)
                    (op)
                    (issued_time)
                    (skip)
                  )
