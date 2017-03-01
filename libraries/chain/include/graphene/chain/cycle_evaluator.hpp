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

    const account_object* _account_obj = nullptr;
  };

} }  // namespace graphene::chain
