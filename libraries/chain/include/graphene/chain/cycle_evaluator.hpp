/**
 * DASCOIN!
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

} }  // namespace graphene::chain
