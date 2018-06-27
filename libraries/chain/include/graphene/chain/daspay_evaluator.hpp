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

#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/daspay_operations.hpp>
#include <graphene/chain/daspay_object.hpp>
#include <fc/noncopyable.hpp>

namespace graphene { namespace chain {

  class set_daspay_transaction_ratio_evaluator : public evaluator<set_daspay_transaction_ratio_evaluator>
  {
  public:
    typedef set_daspay_transaction_ratio_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);
  };

  class register_daspay_authority_evaluator : public evaluator<register_daspay_authority_evaluator>
  {
  public:
    typedef register_daspay_authority_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    object_id_type do_apply( const operation_type& op );
  };

  class unregister_daspay_authority_evaluator : public evaluator<unregister_daspay_authority_evaluator>
  {
  public:
    typedef unregister_daspay_authority_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );

  private:
    const daspay_authority_object* _daspay_authority_obj = nullptr;
  };

  class reserve_asset_on_account_evaluator : public evaluator<reserve_asset_on_account_evaluator>
  {
  public:
    typedef reserve_asset_on_account_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );
  };

  class unreserve_asset_on_account_evaluator : public evaluator<unreserve_asset_on_account_evaluator>
  {
  public:
    typedef unreserve_asset_on_account_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    object_id_type do_apply( const operation_type& op );
  };

  class create_payment_service_provider_evaluator : public evaluator<create_payment_service_provider_evaluator>
  {
  public:
    typedef create_payment_service_provider_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);
  };

  class update_payment_service_provider_evaluator : public evaluator<update_payment_service_provider_evaluator>
  {
  public:
    typedef update_payment_service_provider_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);

  private:
    const payment_service_provider_object* _pspo_to_update = nullptr;
  };

  class payment_service_provider_evaluator_helper : fc::noncopyable
  {
  public:
    explicit payment_service_provider_evaluator_helper(const database& db)
      : _db(db) {}

    template<typename OperationType>
    const payment_service_provider_object* do_evaluate(OperationType op)
    {
      const auto& gpo = _db.get_global_properties();

      const auto& issuer_obj = op.authority(_db);
      _db.perform_chain_authority_check("daspay authority", gpo.authorities.daspay_administrator, issuer_obj);

      FC_ASSERT( op.payment_service_provider_account(_db).is_wallet(),
                 "Account '${name}' must be a wallet account",
                 ("name", op.payment_service_provider_account(_db).name)
      );

      for (const auto& clearing_acc : op.payment_service_provider_clearing_accounts)
        clearing_acc(_db);

      const auto& idx = _db.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
      auto psp_iterator = idx.find(op.payment_service_provider_account);
      if( psp_iterator != idx.end() )
        return &(*psp_iterator);
      return nullptr;
    }

  private:
    const database& _db;
  };

  class delete_payment_service_provider_evaluator : public evaluator<delete_payment_service_provider_evaluator>
  {
  public:
    typedef delete_payment_service_provider_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);

  private:
    const payment_service_provider_object* _pspo_to_delete = nullptr;
  };

  class daspay_debit_account_evaluator : public evaluator<daspay_debit_account_evaluator>
  {
  public:
    typedef daspay_debit_account_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    operation_result do_apply( const operation_type& op );

  private:
    asset _to_debit;
  };

  class daspay_credit_account_evaluator : public evaluator<daspay_credit_account_evaluator>
  {
  public:
    typedef daspay_credit_account_operation operation_type;

    void_result do_evaluate(const operation_type &op);
    operation_result do_apply(const operation_type &op);

  private:
    asset _to_credit;
  };

  class update_daspay_clearing_parameters_evaluator : public evaluator<update_daspay_clearing_parameters_evaluator>
  {
  public:
    typedef update_daspay_clearing_parameters_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);
  };

  class update_delayed_operations_resolver_parameters_evaluator : public evaluator<update_delayed_operations_resolver_parameters_evaluator>
  {
  public:
    typedef update_delayed_operations_resolver_parameters_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);
  };

} }  // namespace graphene::chain
