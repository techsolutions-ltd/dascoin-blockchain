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

#include <fc/smart_ref_impl.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/update_global_parameters_evaluator.hpp>

namespace graphene { namespace chain {

  void_result update_global_parameters_evaluator::do_evaluate(const operation_type &op)
  { try {
    const auto& d = db();
    const auto& gpo = d.get_global_properties();
    const auto& authority_obj = op.authority(d);

    d.perform_chain_authority_check("root authority", gpo.authorities.root_administrator, authority_obj);

    FC_ASSERT( op.new_parameters.block_interval >= 1, "Cannot set block interval to value less than 1" );
    FC_ASSERT( op.new_parameters.maintenance_interval >= op.new_parameters.block_interval, "Cannot set maintenance interval to value less than block interval" );
    FC_ASSERT( op.new_parameters.maintenance_interval % op.new_parameters.block_interval == 0, "Maintenance interval needs to be multiple of block interval" );

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result update_global_parameters_evaluator::do_apply(const operation_type &op)
  { try {
    auto& d = db();

    d.modify(d.get_global_properties(), [&op](global_property_object& gpo) {
      gpo.parameters = op.new_parameters;
      gpo.parameters.apply_fee_asset_id();
    });

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
