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

#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/exceptions.hpp>

namespace graphene { namespace chain {

  class submit_reserve_cycles_to_queue_evaluator : public evaluator<submit_reserve_cycles_to_queue_evaluator>
  {
  public:
    typedef submit_reserve_cycles_to_queue_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);
  };

  class submit_cycles_to_queue_evaluator : public evaluator<submit_cycles_to_queue_evaluator>
  {
  public:
    typedef submit_cycles_to_queue_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);

  private:
    const license_information_object* _license_information_obj = nullptr;
    license_type_id_type _license_type;
  };

  class submit_cycles_to_queue_by_license_evaluator : public evaluator<submit_cycles_to_queue_by_license_evaluator>
  {
  public:
    typedef submit_cycles_to_queue_by_license_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);

  private:
    const license_information_object* _license_information_obj = nullptr;
  };

  class update_queue_parameters_evaluator : public evaluator<update_queue_parameters_evaluator>
  {
  public:
    typedef update_queue_parameters_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);

    const global_property_object* _gpo = nullptr;
  };

  class update_global_frequency_evaluator : public evaluator<update_global_frequency_evaluator>
  {
    public:
      typedef update_global_frequency_operation operation_type;

      void_result do_evaluate(const operation_type& op);
      object_id_type do_apply(const operation_type& op);
  };

  class issue_free_cycles_evaluator : public evaluator<issue_free_cycles_evaluator>
  {
    public:
      typedef issue_free_cycles_operation operation_type;

      void_result do_evaluate(const operation_type& op);
      void_result do_apply(const operation_type& op);

    private:
      const account_cycle_balance_object* _cycle_balance_obj = nullptr;
  };

  class issue_cycles_to_license_evaluator : public evaluator<issue_cycles_to_license_evaluator>
  {
  public:
    typedef issue_cycles_to_license_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);

  private:
    const license_information_object* _license_information_obj = nullptr;
    license_kind _kind;
    frequency_type _frequency_lock;
  };

} }  // namespace graphene::chain
