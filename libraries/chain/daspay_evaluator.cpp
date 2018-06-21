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
    const auto& itr_end = idx.upper_bound(op.issuer);

    FC_ASSERT( std::find_if(itr, itr_end, [&op](const daspay_authority_object& dao) {
      return dao.payment_provider == op.payment_provider;
    } ) == itr_end, "DasPay payment provider ${p} already set", ("p", op.payment_provider) );

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
    const auto& gpo = d.get_global_properties();

    FC_ASSERT( gpo.delayed_operations_resolver_enabled, "Cannot issue unreserve operation because delayed operations resolver is not running" );

    FC_ASSERT( op.asset_to_unreserve.asset_id == d.get_dascoin_asset_id(), "Only dascoin can be unreserved for daspay" );

    const auto& idx = d.get_index_type<delayed_operations_index>().indices().get<by_account>();
    const auto& itr = idx.lower_bound(op.account);

    FC_ASSERT( itr == idx.end(), "Cannot issue another unreserve operation while the previous one is pending ${a}", ("a", itr->id) );

    const auto& balance = d.get_balance_object( op.account, d.get_dascoin_asset_id() );
    const auto& reserved_asset = asset{ balance.reserved, d.get_dascoin_asset_id() };

    FC_ASSERT( op.asset_to_unreserve.amount <= balance.reserved, "Cannot unreserve ${a} because there is only ${b} left", ("a", d.to_pretty_string(op.asset_to_unreserve))("b", d.to_pretty_string(reserved_asset)) );

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type unreserve_asset_on_account_evaluator::do_apply(const operation_type& op)
  { try {
    database& d = db();
    const auto& gpo = d.get_global_properties();

    return d.create<delayed_operation_object>([&](delayed_operation_object& duo) {
      duo.account = op.account;
      duo.op = op;
      duo.issued_time = d.head_block_time();
      duo.skip = gpo.delayed_operations_resolver_interval_time_seconds;
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result unregister_daspay_authority_evaluator::do_evaluate(const operation_type& op)
  { try {
    const auto& d = db();
    const auto& idx = d.get_index_type<daspay_authority_index>().indices().get<by_daspay_user>();

    auto itr = idx.lower_bound(op.issuer);
    FC_ASSERT( itr != idx.end(), "Cannot unregister DasPay authority because none has been set" );

    const auto& itr_end = idx.upper_bound(op.issuer);
    auto it = std::find_if(itr, itr_end, [&op](const daspay_authority_object& dao){
      return dao.payment_provider == op.payment_provider;
    });

    if (it != idx.end())
    {
      _daspay_authority_obj = &(*it);
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

    FC_ASSERT( op.payment_service_provider_account(d).is_wallet(),
               "Account '${name}' must be a wallet account",
               ("name", op.payment_service_provider_account(d).name)
    );

    for (const auto& clearing_acc : op.payment_service_provider_clearing_accounts)
      clearing_acc(d);

    const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    auto psp_iterator = idx.find(op.payment_service_provider_account);
    FC_ASSERT( psp_iterator != idx.end(), "Payment service provider with account ${1} doesn't exists.", ("1", op.payment_service_provider_account));
    _pspo_to_update = &(*psp_iterator);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result update_payment_service_provider_evaluator::do_apply(const update_payment_service_provider_operation& op)
  { try {
    auto& d = db();

    d.modify(*_pspo_to_update, [&](payment_service_provider_object& pspo) {
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

    const auto& idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    auto psp_iterator = idx.find(op.payment_service_provider_account);
    FC_ASSERT( psp_iterator != idx.end(), "Payment service provider with account ${1} doesn't exists.", ("1", op.payment_service_provider_account));
    _pspo_to_delete = &(*psp_iterator);

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result delete_payment_service_provider_evaluator::do_apply(const delete_payment_service_provider_operation& op)
  { try {
    auto& d = db();

    d.remove(*_pspo_to_delete);

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_debit_account_evaluator::do_evaluate(const daspay_debit_account_operation& op)
  { try {
    const auto& d = db();

    const auto& delayed_unreserve_idx = d.get_index_type<delayed_operations_index>().indices().get<by_account>();
    auto delayed_unreserve_iterator = delayed_unreserve_idx.find(op.account);
    FC_ASSERT( delayed_unreserve_iterator == delayed_unreserve_idx.end(), "Account ${1} initiated delayed unreserve operation.", ("1", op.account) );

    FC_ASSERT( op.debit_amount.asset_id == d.get_web_asset_id(), "Only web euro can be debited, ${a} sent", ("a", d.to_pretty_string(op.debit_amount)) );

    const auto& account = op.account(d);
    FC_ASSERT( account.is_wallet(), "Cannot debit vault account ${i}", ("i", op.account) );

    const auto& da_idx = d.get_index_type<daspay_authority_index>().indices().get<by_daspay_user>();
    FC_ASSERT( da_idx.find(op.account) != da_idx.end(), "Cannot debit user who has not enabled daspay" );

    const auto& da_it = da_idx.lower_bound(op.account);
    const auto& da_itr_end = da_idx.upper_bound(op.account);
    FC_ASSERT( std::find_if(da_it, da_itr_end, [&op](const daspay_authority_object& dao) { return dao.daspay_public_key == op.auth_key; } ) != da_idx.end(), "Trying to sign debit operation with the key user has not authorized" );

    const auto& psp_idx = d.get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
    const auto& psp_it = psp_idx.find(op.payment_service_provider_account);
    FC_ASSERT( psp_it != psp_idx.end(), "Payment service provider with account ${1} does not exist.", ("1", op.payment_service_provider_account) );

    FC_ASSERT( std::find(psp_it->payment_service_provider_clearing_accounts.begin(),
                         psp_it->payment_service_provider_clearing_accounts.end(),
                         op.clearing_account) != psp_it->payment_service_provider_clearing_accounts.end(), "Invalid clearing account" );

    const auto& balance = d.get_balance_object(op.account, d.get_dascoin_asset_id());
    const auto& dgpo = d.get_dynamic_global_properties();
    decltype(op.debit_amount) tmp{op.debit_amount};
    tmp.amount += tmp.amount * dgpo.daspay_debit_transaction_ratio / 10000;
    _to_debit = tmp * dgpo.last_dascoin_price;

    const asset reserved{balance.reserved, d.get_dascoin_asset_id()};
    FC_ASSERT( _to_debit <= reserved, "Not enough reserved balance on user account ${a}, left ${l}, needed ${n}", ("a", op.account)("l", d.to_pretty_string(reserved))("n", d.to_pretty_string(_to_debit)) );

    return {};
  } FC_CAPTURE_AND_RETHROW((op)) }

  operation_result daspay_debit_account_evaluator::do_apply(const daspay_debit_account_operation& op)
  { try {
    auto& d = db();

    d.adjust_balance(op.account, asset{0, _to_debit.asset_id}, -_to_debit.amount);
    d.adjust_balance(op.clearing_account, _to_debit, 0);

    return _to_debit;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result daspay_credit_account_evaluator::do_evaluate(const operation_type& op)
  { try {
    const auto& d = db();

    FC_ASSERT( op.credit_amount.asset_id == d.get_web_asset_id(), "Only webeur can be credited, ${a} sent", ("a", d.to_pretty_string(op.credit_amount)) );

    const auto& account = op.account(d);
    FC_ASSERT( account.is_wallet(), "Cannot credit vault account ${i}", ("i", op.account) );

    const auto& da_idx = d.get_index_type<daspay_authority_index>().indices().get<by_daspay_user>();
    const auto& da_it = da_idx.find(op.account);

    FC_ASSERT( da_it != da_idx.end(), "Cannot credit user who has not enabled daspay" );

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
    _to_credit = tmp * dgpo.last_dascoin_price;

    FC_ASSERT( _to_credit <= balance, "Not enough balance on clearing account ${a}, left ${l}, needed ${n}", ("a", op.clearing_account)("l", d.to_pretty_string(balance))("n", d.to_pretty_string(_to_credit)) );

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  operation_result daspay_credit_account_evaluator::do_apply(const operation_type& op)
  { try {
    auto& d = db();

    d.adjust_balance(op.clearing_account, asset{-_to_credit.amount, _to_credit.asset_id}, 0);
    d.adjust_balance(op.account, asset{0, _to_credit.asset_id}, _to_credit.amount);

    return _to_credit;

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result update_daspay_clearing_parameters_evaluator::do_evaluate(const update_daspay_clearing_parameters_operation& op)
  { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();
    const auto& authority_obj = op.authority(d);

    d.perform_chain_authority_check("daspay authority", gpo.authorities.daspay_administrator, authority_obj);

    if ( op.clearing_interval_time_seconds.valid() )
    {
      FC_ASSERT( *op.clearing_interval_time_seconds % gpo.parameters.block_interval == 0,
                "Clearing interval must be a multiple of the block interval ${bi}",
                ("bi", gpo.parameters.block_interval)
      );

      FC_ASSERT( *op.clearing_interval_time_seconds >= 2 * gpo.parameters.block_interval,
                 "Clearing interval must be greater or equal to double of block interval ${bi}",
                 ("bi", gpo.parameters.block_interval)
      );
    }
    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result update_daspay_clearing_parameters_evaluator::do_apply(const update_daspay_clearing_parameters_operation& op)
  { try {
    auto& d = db();

    d.modify(d.get_global_properties(), [&](global_property_object& gpo){
      CHECK_AND_SET_OPT(gpo.daspay_parameters.clearing_enabled, op.clearing_enabled);
      CHECK_AND_SET_OPT(gpo.daspay_parameters.clearing_interval_time_seconds, op.clearing_interval_time_seconds);
      CHECK_AND_SET_OPT(gpo.daspay_parameters.collateral_dascoin, op.collateral_dascoin);
      CHECK_AND_SET_OPT(gpo.daspay_parameters.collateral_webeur, op.collateral_webeur);
    });

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result update_delayed_operations_resolver_parameters_evaluator::do_evaluate(const update_delayed_operations_resolver_parameters_operation& op)
  { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();
    const auto& authority_obj = op.authority(d);

    d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, authority_obj);

    if ( op.delayed_operations_resolver_interval_time_seconds.valid() )
    {
      FC_ASSERT( *op.delayed_operations_resolver_interval_time_seconds % gpo.parameters.block_interval == 0,
                 "Delayed operations resolver interval must be a multiple of the block interval ${bi}",
                 ("bi", gpo.parameters.block_interval)
      );

    }
    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  void_result update_delayed_operations_resolver_parameters_evaluator::do_apply(const update_delayed_operations_resolver_parameters_operation& op)
  { try {
    auto& d = db();

    d.modify(d.get_global_properties(), [&](global_property_object& gpo){
      CHECK_AND_SET_OPT(gpo.delayed_operations_resolver_enabled, op.delayed_operations_resolver_enabled);
      CHECK_AND_SET_OPT(gpo.delayed_operations_resolver_interval_time_seconds, op.delayed_operations_resolver_interval_time_seconds);
    });

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
