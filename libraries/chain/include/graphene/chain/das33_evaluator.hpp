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
#include <graphene/chain/protocol/das33_operations.hpp>
#include <graphene/chain/das33_object.hpp>

namespace graphene { namespace chain {

  class das33_project_create_evaluator : public evaluator<das33_project_create_evaluator>
  {
  public:
    typedef das33_project_create_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    object_id_type do_apply( const operation_type& op );
  };

  class das33_project_update_evaluator : public evaluator<das33_project_update_evaluator>
  {
  public:
    typedef das33_project_update_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );
  private:
    const das33_project_object* project_to_update = nullptr;
  };

  class das33_project_delete_evaluator : public evaluator<das33_project_delete_evaluator>
  {
  public:
    typedef das33_project_delete_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );
  private:
    const das33_project_object* project_to_delete = nullptr;
  };

  class das33_pledge_asset_evaluator : public evaluator<das33_pledge_asset_evaluator>
  {
  public:
    typedef das33_pledge_asset_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    object_id_type do_apply( const operation_type& op );

  private:
    asset total;
    asset base;
    asset bonus;
    asset to_take;
    price price_at_evaluation;
    share_type discount;
  };

  class das33_distribute_project_pledges_evaluator : public evaluator<das33_distribute_project_pledges_evaluator>
  {
  public:
    typedef das33_distribute_project_pledges_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );

  private:
        account_id_type _pro_owner;
  };

  class das33_project_reject_evaluator : public evaluator<das33_project_reject_evaluator>
  {
  public:
    typedef das33_project_reject_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );

  private:
      account_id_type _pro_owner;
  };

  class das33_distribute_pledge_evaluator : public evaluator<das33_distribute_pledge_evaluator>
  {
  public:
    typedef das33_distribute_pledge_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );

  private:
    const das33_pledge_holder_object* _pledge_holder_ptr = nullptr;
    account_id_type _pro_owner;
  };

  class das33_pledge_reject_evaluator : public evaluator<das33_pledge_reject_evaluator>
  {
  public:
    typedef das33_pledge_reject_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );

  private:
    const das33_pledge_holder_object* _pledge_holder_ptr = nullptr;
    account_id_type _pro_owner;
  };

  class das33_set_use_external_btc_price_evaluator : public evaluator<das33_set_use_external_btc_price_evaluator>
  {
  public:
    typedef das33_set_use_external_btc_price_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );
  };

  class das33_set_use_market_price_for_token_evaluator : public evaluator<das33_set_use_market_price_for_token_evaluator>
  {
  public:
    typedef das33_set_use_market_price_for_token_operation operation_type;

    void_result do_evaluate( const operation_type& op );
    void_result do_apply( const operation_type& op );
  };

  asset asset_price_multiply ( const asset& a, int64_t precision, const price& b, const price& c );
  share_type precision_modifier(asset_object a, asset_object b);
  price get_price_in_web_eur(asset_id_type original_asset_id, const database& d);


} }  // namespace graphene::chain
