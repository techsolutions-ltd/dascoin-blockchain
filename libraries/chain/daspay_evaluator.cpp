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

#include <graphene/chain/daspay_evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

  void_result set_daspay_transaction_ratio_evaluator::do_evaluate(const set_daspay_transaction_ratio_operation& op)
  { try {
      const auto& d = db();
      const auto& gpo = d.get_global_properties();
      const auto& issuer_obj = op.authority(d);

      d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, issuer_obj);

      return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result set_daspay_transaction_ratio_evaluator::do_apply(const set_daspay_transaction_ratio_operation& op)
  { try {
      auto& d = db();

      d.modify(d.get_dynamic_global_properties(), [&op](dynamic_global_property_object& dgpo){
        dgpo.daspay_debit_transaction_ratio = op.debit_ratio;
        dgpo.daspay_credit_transaction_ratio = op.credit_ratio;
      });

      return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result register_daspay_authority_evaluator::do_evaluate(const operation_type& op)
  {
    try {
      const auto& d = db();
      const auto& issuer = op.issuer(d);
      FC_ASSERT( issuer.is_wallet(), "Cannot register DasPay authority on vault ${i}", ("i", op.issuer) );

      const auto& payment_provider = op.payment_provider(d);
      FC_ASSERT( payment_provider.is_wallet(), "Cannot register DasPay authority because payment provider ${p} is not wallet", ("p", op.payment_provider) );

      const auto& psp_idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
      FC_ASSERT( psp_idx.find(op.payment_provider) != psp_idx.end(), "Cannot add DasPay authority because payment provider is not registered" );

      const auto& idx = d.get_index_type<daspay_authority_index>().indices().get<by_daspay_user>();
      auto itr = idx.lower_bound(op.issuer);

      while( itr != idx.end() )
      {
        FC_ASSERT ( itr->payment_provider != op.payment_provider, "DasPay payment provider ${p} already set", ("p", op.payment_provider) );
        ++itr;
      }
      return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type register_daspay_authority_evaluator::do_apply(const operation_type& op)
  {
    try {
      auto& d = db();
      return d.create<daspay_authority_object>([&](daspay_authority_object& o){
        o.daspay_user = op.issuer;
        o.payment_provider = op.payment_provider;
        o.daspay_public_key = op.daspay_public_key;
        o.memo = op.memo;
      }).id;
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result unregister_daspay_authority_evaluator::do_evaluate(const operation_type& op)
  {
    try {
      const auto& d = db();
      const auto& idx = d.get_index_type<daspay_authority_index>().indices().get<by_daspay_user>();

      auto itr = idx.lower_bound(op.issuer);
      FC_ASSERT( itr != idx.end(), "Cannot unregister DasPay authority because none has been set" );

      while( itr != idx.end() )
      {
        if ( itr->payment_provider == op.payment_provider )
        {
          const auto& obj = *itr;
          _daspay_authority_obj = &obj;
          break;
        }
        ++itr;
      }

      FC_ASSERT( _daspay_authority_obj != nullptr, "Cannot unregister DasPay authority ${a} since ${u} is not the owner", ("a", op.payment_provider)("u", op.issuer) );

      return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result unregister_daspay_authority_evaluator::do_apply(const operation_type& op)
  {
    try {
      auto& d = db();
      d.remove(*_daspay_authority_obj);
      return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_debit_evaluator::do_evaluate(const operation_type& op)
  {
    try {
      const auto& d = db();
      _account_obj = &op.issuer(d);

      return {};
    } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_debit_evaluator::do_apply(const operation_type& op)
  { try {

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

 void_result create_payment_service_provider_evaluator::do_evaluate(const create_payment_service_provider_operation& op)
 { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();

    const auto& issuer_obj = op.authority(d);
    d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, issuer_obj);

    op.payment_service_provider_account(d);
    for (const auto& clearing_acc : op.payment_service_provider_clearing_accounts)
      clearing_acc(d);

     const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
     FC_ASSERT(idx.find(op.payment_service_provider_account) == idx.end(), "Payment service provider with account ${1} already exists.", ("1", op.payment_service_provider_account));

     return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

 object_id_type create_payment_service_provider_evaluator::do_apply(const create_payment_service_provider_operation& op)
 { try {
     auto& d = db();

     return d.create<payment_service_provider_object>([&](payment_service_provider_object& pspo){
       pspo.payment_service_provider_account = op.payment_service_provider_account;
       pspo.payment_service_provider_clearing_accounts = op.payment_service_provider_clearing_accounts;
     }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
