/*
 * MIT License
 *
 * Copyright (c) 2018 TechSolutions Ltd.
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

#include <fc/noncopyable.hpp>

namespace graphene { namespace chain { namespace detail {

  class submit_cycles_evaluator_helper : fc::noncopyable
  {
  public:
    explicit submit_cycles_evaluator_helper(const database& db)
      : _db{db}{}

    template<typename OperationType>
    const license_information_object* do_evaluate(const OperationType& op, const license_type_id_type &license_type,
                                                  frequency_type frequency) const
    {
      const auto& account_obj = op.account(_db);

      // Only vault accounts are allowed to submit cycles:
      FC_ASSERT( account_obj.is_vault(),
                 "Account '${n}' is not a vault account",
                 ("n", account_obj.name)
               );

      FC_ASSERT( account_obj.license_information.valid(),
                 "Cannot submit cycles, account '${n}' does not have any licenses",
                 ("n", account_obj.name)
               );

      const auto& license_information_obj = (*account_obj.license_information)(_db);

      // Check if this account has a license of required type:
      optional<license_information_object::license_history_record> license{license_information_obj.get_license(license_type)};

      FC_ASSERT( license.valid(), "Account '${n}' does not have a license of type ${l}",
                 ("n", account_obj.name)
                 ("l", license_type)
               );

      // Assure we have enough funds to submit:
      FC_ASSERT( license->total_cycles() >= op.amount,
                 "Cannot submit ${am} cycles, account '${n}' license cycle balance is ${b}",
                 ("am", op.amount)
                 ("n", account_obj.name)
                 ("b", license->amount)
               );

      // Assure frequency submitted is the same as in the license:
      FC_ASSERT( license->frequency_lock == frequency,
                 "Cannot submit ${am} cycles, frequency set (${f}) is not equal to license's frequency (${lf})",
                 ("am", op.amount)
                 ("f", frequency)
                 ("lf", license->frequency_lock)
               );

      return &license_information_obj;
    }

    template<typename OperationType>
    object_id_type do_apply(const OperationType& op, const license_information_object* lio,
                            const license_type_id_type &license_type, frequency_type frequency)
    {
      auto& d = const_cast<database &>(_db);

      // Spend cycles, decrease balance and supply:
      d.reserve_cycles(op.account, op.amount);
      auto origin = fc::reflector<dascoin_origin_kind>::to_string(user_submit);
      d.modify(*lio, [&](license_information_object& lio){
        lio.subtract_cycles(license_type, op.amount);
      });

      return d.push_queue_submission(origin, license_type, op.account, op.amount, frequency, op.comment);
    }

  private:
    const database& _db;
  };

} } }  // namespace graphene::chain::detail
