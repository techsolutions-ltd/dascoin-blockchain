/**
 * DASCOIN!
 */
#pragma once

#include <graphene/chain/upgrade_event_object.hpp>
#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {

  class create_upgrade_event_evaluator : public evaluator<create_upgrade_event_evaluator>
  {
  public:
    using operation_type = create_upgrade_event_operation;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);

  private:
  };

} }  // namespace graphene::chain
