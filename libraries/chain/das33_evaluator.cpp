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

#include <graphene/chain/das33_evaluator.hpp>
#include <graphene/chain/database.hpp>

namespace graphene { namespace chain {

  void_result das33_pledge_cycles_evaluator::do_evaluate(const das33_pledge_cycles_operation& op)
  { try {

    const auto& d = db();
    const auto& account_obj = op.vault_id(d);

    // Only vault accounts are allowed to submit cycles:
    FC_ASSERT( account_obj.is_vault(),
               "Account '${n}' is not a vault account",
               ("n", account_obj.name)
             );

    FC_ASSERT( account_obj.license_information.valid(),
               "Cannot submit cycles, account '${n}' does not have any licenses",
               ("n", account_obj.name)
          );

    const auto& license_information_obj = (*account_obj.license_information)(d);

    // Check if this account has a required license:
    optional<license_information_object::license_history_record> license{license_information_obj.get_license(op.license_id)};
    FC_ASSERT( license.valid(),
               "Account '${n}' does not have a license of type ${l}",
               ("n", account_obj.name)
               ("l", op.license_id)
             );

    // Assure we have enough funds to submit:
    FC_ASSERT( license->total_cycles() >= op.cycles_amount,
               "Cannot submit ${am} cycles, account '${n}' license cycle balance is ${b}",
               ("am", op.cycles_amount)
               ("n", account_obj.name)
               ("b", license->amount)
          );

    //TODO
    // Assure that amount of cycles submitted would not exceed token max supply limit.
    //const auto& project_obj = op.project_id(d);
    //FC_ASSERT(d.cycles_to_asset(op.cycles_amount, frequency, project_obj.token_id) + project_obj.collected <= project_obj.token_id.max_supply * token_precision(project_obj.token_id),
    //            "Cannot submit ${am} cycles with frequency (${f}), "
    //            "because it would exceed token's max supply limit ${max_limit}",
    //            ("am", op.amount)
    //            ("f", frequency)
    //            ("max_limit", project_obj.token_id.max_supply * token_precision(project_obj.token_id))
    //          );

    return {};

  } FC_CAPTURE_AND_RETHROW((op)) }

  object_id_type das33_pledge_cycles_evaluator::do_apply(const das33_pledge_cycles_operation& op)
  { try {

    auto& d = db();
    const auto& account_obj = op.vault_id(d);
    const auto& license_information_obj = (*account_obj.license_information)(d);

    // Spend cycles, decrease balance and supply:
    d.reserve_cycles(op.vault_id, op.cycles_amount);
    d.modify( license_information_obj, [&](license_information_object& lio){
              lio.subtract_cycles(op.license_id, op.cycles_amount);
            });

    // Create the holder object and return its ID:
    return d.create<das33_cycles_pledge_holder_object>([&](das33_cycles_pledge_holder_object& cpho){
        cpho.vault_id = op.vault_id;
        cpho.license_id = op.license_id;
        cpho.cycles_amount = op.cycles_amount;
        cpho.token_amount = 0;
        cpho.project_id = op.project_id;
        cpho.timestamp = d.head_block_time();
    }).id;

  } FC_CAPTURE_AND_RETHROW((op)) }

} }  // namespace graphene::chain
