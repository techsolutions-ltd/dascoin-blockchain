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

    d.perform_chain_authority_check("daspay authority", gpo.authorities.daspay_administrator, issuer_obj);

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
  { try {
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
  { try {
    auto& d = db();

    return d.create<daspay_authority_object>([&](daspay_authority_object& o){
      o.daspay_user = op.issuer;
      o.payment_provider = op.payment_provider;
      o.daspay_public_key = op.daspay_public_key;
      o.memo = op.memo;
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result reserve_asset_on_account_evaluator::do_evaluate(const operation_type& op)
  { try {
    const database& d = db();

    FC_ASSERT( op.asset_to_reserve.asset_id == d.get_dascoin_asset_id(), "Only dascoin can be reserved for daspay" );

    const auto& balance = d.get_balance( op.account, d.get_dascoin_asset_id() );

    FC_ASSERT( op.asset_to_reserve.amount <= balance.amount, "Cannot reserve ${a} because there is only ${b} left", ("a", d.to_pretty_string(op.asset_to_reserve))("b", d.to_pretty_string(balance)) );

      return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result reserve_asset_on_account_evaluator::do_apply(const operation_type& op)
  { try {
    database& d = db();
    d.adjust_balance( op.account, asset{-op.asset_to_reserve.amount, d.get_dascoin_asset_id()}, op.asset_to_reserve.amount );

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result unreserve_asset_on_account_evaluator::do_evaluate(const operation_type& op)
  { try {
    const database& d = db();

    FC_ASSERT( op.asset_to_unreserve.asset_id == d.get_dascoin_asset_id(), "Only dascoin can be unreserved for daspay" );

    const auto& balance = d.get_balance_object( op.account, d.get_dascoin_asset_id() );
    const auto& reserved_asset = asset{ balance.reserved, d.get_dascoin_asset_id() };

    FC_ASSERT( op.asset_to_unreserve.amount <= balance.reserved, "Cannot unreserve ${a} because there is only ${b} left", ("a", d.to_pretty_string(op.asset_to_unreserve))("b", d.to_pretty_string(reserved_asset)) );

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result unreserve_asset_on_account_evaluator::do_apply(const operation_type& op)
  { try {
    database& d = db();
    d.adjust_balance( op.account, asset{op.asset_to_unreserve.amount, d.get_dascoin_asset_id()}, -op.asset_to_unreserve.amount );

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result unregister_daspay_authority_evaluator::do_evaluate(const operation_type& op)
  { try {
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
  { try {
      auto& d = db();

      d.remove(*_daspay_authority_obj);

      return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result create_payment_service_provider_evaluator::do_evaluate(const create_payment_service_provider_operation& op)
  { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();

    const auto& issuer_obj = op.authority(d);
    d.perform_chain_authority_check("daspay authority", gpo.authorities.daspay_administrator, issuer_obj);

    op.payment_service_provider_account(d);
    FC_ASSERT( op.payment_service_provider_account(d).is_wallet(),
               "Account '${name}' must be a wallet account",
               ("name", op.payment_service_provider_account(d).name)
    );

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

  void_result update_payment_service_provider_evaluator::do_evaluate(const update_payment_service_provider_operation& op)
  { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();

    const auto& issuer_obj = op.authority(d);
    d.perform_chain_authority_check("daspay authority", gpo.authorities.daspay_administrator, issuer_obj);

    op.payment_service_provider_account(d);
    for (const auto& clearing_acc : op.payment_service_provider_clearing_accounts)
      clearing_acc(d);

    const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    auto psp_iterator = idx.find(op.payment_service_provider_account);
    FC_ASSERT( psp_iterator != idx.end(), "Payment service provider with account ${1} doesn't exists.", ("1", op.payment_service_provider_account));
    pspo_to_update = &(*psp_iterator);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result update_payment_service_provider_evaluator::do_apply(const update_payment_service_provider_operation& op)
  { try {
    auto& d = db();

    d.modify(*pspo_to_update, [&](payment_service_provider_object& pspo) {
      pspo.payment_service_provider_account = op.payment_service_provider_account;
      pspo.payment_service_provider_clearing_accounts = op.payment_service_provider_clearing_accounts;
    });

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result delete_payment_service_provider_evaluator::do_evaluate(const delete_payment_service_provider_operation& op)
  { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();
    const auto& issuer_obj = op.authority(d);

    d.perform_chain_authority_check("daspay authority", gpo.authorities.daspay_administrator, issuer_obj);

    op.payment_service_provider_account(d);

    const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    auto psp_iterator = idx.find(op.payment_service_provider_account);
    FC_ASSERT( psp_iterator != idx.end(), "Payment service provider with account ${1} doesn't exists.", ("1", op.payment_service_provider_account));
    pspo_to_delete = &(*psp_iterator);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result delete_payment_service_provider_evaluator::do_apply(const delete_payment_service_provider_operation& op)
  { try {
    auto& d = db();

    d.remove(*pspo_to_delete);

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_debit_account_evaluator::do_evaluate(const daspay_debit_account_operation& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.debit_amount.asset_id == d.get_web_asset_id(), "Only dascoin can be debited, ${a} sent", ("a", d.to_pretty_string(op.debit_amount)) );

    const auto& account = op.account(d);
    FC_ASSERT( account.is_wallet(), "Cannot debit vault account ${i}", ("i", op.account) );

    const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    const auto& it = idx.find(op.payment_service_provider_account);
    FC_ASSERT( it != idx.end(), "Payment service provider with account ${1} does not exist.", ("1", op.payment_service_provider_account) );

    FC_ASSERT( std::find(it->payment_service_provider_clearing_accounts.begin(),
                         it->payment_service_provider_clearing_accounts.end(),
                         op.clearing_account) != it->payment_service_provider_clearing_accounts.end(), "Invalid clearing account" );

    const auto& balance = d.get_balance(op.clearing_account, d.get_dascoin_asset_id());
    const auto& dgpo = d.get_dynamic_global_properties();
    decltype(op.debit_amount) tmp{op.debit_amount};
    tmp.amount += tmp.amount * dgpo.daspay_credit_transaction_ratio / 10000;
    to_debit = tmp * dgpo.last_dascoin_price;

    FC_ASSERT( to_debit <= balance, "Not enough balance on user account ${a}, left ${l}, needed ${n}", ("a", op.account)("l", d.to_pretty_string(balance))("n", d.to_pretty_string(to_debit)) );

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_debit_account_evaluator::do_apply(const daspay_debit_account_operation& op)
  { try {
    auto& d = db();

    d.adjust_balance(op.account, asset{-to_debit.amount, to_debit.asset_id}, 0);
    d.adjust_balance(op.clearing_account, asset{0, to_debit.asset_id}, to_debit.amount);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_credit_account_evaluator::do_evaluate(const operation_type& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.credit_amount.asset_id == d.get_web_asset_id(), "Only webeur can be credited, ${a} sent", ("a", d.to_pretty_string(op.credit_amount)) );

    const auto& account = op.account(d);
    FC_ASSERT( account.is_wallet(), "Cannot credit vault account ${i}", ("i", op.account) );

    const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    const auto& it = idx.find(op.payment_service_provider_account);
    FC_ASSERT( it != idx.end(), "Payment service provider with account ${1} does not exist.", ("1", op.payment_service_provider_account) );

    FC_ASSERT( std::find(it->payment_service_provider_clearing_accounts.begin(),
                         it->payment_service_provider_clearing_accounts.end(),
                         op.clearing_account) != it->payment_service_provider_clearing_accounts.end(), "Invalid clearing account" );

    const auto& balance = d.get_balance(op.clearing_account, d.get_dascoin_asset_id());
    const auto& dgpo = d.get_dynamic_global_properties();
    decltype(op.credit_amount) tmp{op.credit_amount};
    tmp.amount += tmp.amount * dgpo.daspay_credit_transaction_ratio / 10000;
    to_credit = tmp * dgpo.last_dascoin_price;

    FC_ASSERT( to_credit <= balance, "Not enough balance on clearing account ${a}, left ${l}, needed ${n}", ("a", op.clearing_account)("l", d.to_pretty_string(balance))("n", d.to_pretty_string(to_credit)) );

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_credit_account_evaluator::do_apply(const operation_type& op)
  { try {
    auto& d = db();

    d.adjust_balance(op.clearing_account, asset{-to_credit.amount, to_credit.asset_id}, 0);
    d.adjust_balance(op.account, asset{0, to_credit.asset_id}, to_credit.amount);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
