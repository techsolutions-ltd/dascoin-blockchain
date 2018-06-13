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
    void_result do_apply( const operation_type& op );
  };

  class create_payment_service_provider_evaluator : public evaluator<create_payment_service_provider_evaluator>
  {
  public:
    typedef create_payment_service_provider_operation operation_type;

    void_result do_evaluate(const create_payment_service_provider_operation& op);
    object_id_type do_apply(const create_payment_service_provider_operation& op);
  };

  class update_payment_service_provider_evaluator : public evaluator<update_payment_service_provider_evaluator>
  {
  public:
    typedef update_payment_service_provider_operation operation_type;

    void_result do_evaluate(const update_payment_service_provider_operation& op);
    void_result do_apply(const update_payment_service_provider_operation& op);

  private:
    const payment_service_provider_object* pspo_to_update = nullptr;
  };

  class delete_payment_service_provider_evaluator : public evaluator<delete_payment_service_provider_evaluator>
  {
  public:
    typedef delete_payment_service_provider_operation operation_type;

    void_result do_evaluate(const delete_payment_service_provider_operation& op);
    void_result do_apply(const delete_payment_service_provider_operation& op);

  private:
    const payment_service_provider_object* pspo_to_delete = nullptr;
  };

  class daspay_debit_account_evaluator : public evaluator<daspay_debit_account_evaluator>
  {
  public:
    typedef daspay_debit_account_operation operation_type;

    void_result do_evaluate( const daspay_debit_account_operation& op );
    operation_result do_apply( const daspay_debit_account_operation& op );

  private:
    asset to_debit;
  };

  class daspay_credit_account_evaluator : public evaluator<daspay_credit_account_evaluator>
  {
  public:
    typedef daspay_credit_account_operation operation_type;

    void_result do_evaluate(const operation_type &op);
    operation_result do_apply(const operation_type &op);

  private:
    asset to_credit;
  };

  class update_daspay_clearing_parameters_evaluator : public evaluator<update_daspay_clearing_parameters_evaluator>
  {
  public:
    typedef update_daspay_clearing_parameters_operation operation_type;

    void_result do_evaluate(const update_daspay_clearing_parameters_operation& op);
    void_result do_apply(const update_daspay_clearing_parameters_operation& op);
  };

} }  // namespace graphene::chain
