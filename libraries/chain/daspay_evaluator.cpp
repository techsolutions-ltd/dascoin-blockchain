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

        d.perform_chain_authority_check("license issuer", gpo.authorities.license_issuer, issuer_obj);

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
        _account_obj = &op.issuer(d);
        return {};
    } FC_CAPTURE_AND_RETHROW((op)) }

    void_result register_daspay_authority_evaluator::do_apply(const operation_type& op)
    {
      try {
        auto& d = db();
        d.modify(*_account_obj, [&](account_object& ao) {
          ao.options.daspay_key = op.daspay_public_key;
        });
        return {};
    } FC_CAPTURE_AND_RETHROW((op)) }

    void_result daspay_debit_evaluator::do_evaluate(const operation_type& op)
    {
      try {
        const auto& d = db();
        _account_obj = &op.issuer(d);
        FC_ASSERT( op.auth_key == _account_obj->options.daspay_key, "Invalid key used to sign daspay_debit");
        return {};
      } FC_CAPTURE_AND_RETHROW((op)) }

    void_result daspay_debit_evaluator::do_apply(const operation_type& op)
    {
      try {
        auto& d = db();
//    d.modify(*_account_obj, [&](account_object& ao) {
//      ao.options.daspay_key = op.daspay_public_key;
//    });
        return {};
      } FC_CAPTURE_AND_RETHROW((op)) }

  } }  // namespace graphene::chain
