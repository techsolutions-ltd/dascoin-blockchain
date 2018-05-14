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

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/personal_identity_evaluator.hpp>

namespace graphene { namespace chain {

void_result update_pi_limits_evaluator::do_evaluate(const update_pi_limits_operation& op)
{ try {

  const database& d = db();
  const auto gpo = d.get_global_properties();

  account_id_type pi_validator_id = gpo.authorities.pi_validator;
  const account_object& account = op.account(d);

  FC_ASSERT( pi_validator_id == op.pi_validator );  // This is signed by the current validator id.

  // TODO: check if the level is OK?

  if( op.new_limits.valid() )
  {
    auto min = gpo.parameters.minimum_transfer_limit;
    auto max = gpo.parameters.maximum_transfer_limit;
    for ( auto limit : *op.new_limits )
    {
      FC_ASSERT( limit > min && limit < max );
    }
  }
  acnt = &account;

  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_pi_limits_evaluator::do_apply(const update_pi_limits_operation& op)
{ try {
  auto& d = db();
  // Update the levels and the limits on the account:
  db().modify(*acnt, [&](account_object& a) {
    a.pi_level = op.level;
    if ( op.new_limits.valid() )
      a.limits = *op.new_limits;
  });

  if ( acnt->is_vault() )
  {
    // For each parent check if the parents pi_level is less then the updated one and, in that case, update the
    // pi_level and the limits.
    for ( auto parent_id : acnt->parents )
    {
      const auto& parent = parent_id(d);
      if ( parent.pi_level < op.level )
        d.modify(parent, [&](account_object& a){
          a.pi_level = op.level;
          if ( op.new_limits.valid() )
            a.limits = *op.new_limits;
        });
    }
  }
  return void_result();

} FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain

