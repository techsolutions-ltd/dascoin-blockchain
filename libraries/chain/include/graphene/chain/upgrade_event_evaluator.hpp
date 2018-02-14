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
  };

  class update_upgrade_event_evaluator : public evaluator<update_upgrade_event_evaluator>
  {
  public:
    using operation_type = update_upgrade_event_operation;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);

  private:
    const upgrade_event_object* _upgrade_event = nullptr;
  };

  class delete_upgrade_event_evaluator : public evaluator<delete_upgrade_event_evaluator>
  {
  public:
    using operation_type = delete_upgrade_event_operation;

    void_result do_evaluate(const operation_type& op);
    void_result do_apply(const operation_type& op);

  private:
    const upgrade_event_object* _upgrade_event = nullptr;
  };

} } // namespace graphene::chain
