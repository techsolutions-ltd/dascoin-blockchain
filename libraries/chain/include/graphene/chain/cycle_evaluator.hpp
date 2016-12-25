/**
 * DASCOIN!
 */
#pragma once

#include <graphene/chain/database.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/exceptions.hpp>

namespace graphene { namespace chain {

  class cycle_issue_request_evaluator : public evaluator<cycle_issue_request_evaluator>
  {
  public:
    typedef cycle_issue_request_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);
  };

  class cycle_issue_deny_evaluator : public evaluator<cycle_issue_deny_evaluator>
  {
  public:
    typedef cycle_issue_deny_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);

    const cycle_issue_request_object* request_ = nullptr;
  };

  class submit_cycles_evaluator : public evaluator<submit_cycles_evaluator>
  {
  public:
    typedef submit_cycles_operation operation_type;

    void_result do_evaluate(const operation_type& op);
    object_id_type do_apply(const operation_type& op);

    const account_object* account_obj_ = nullptr;
    const account_cycle_balance_object* balance_obj_ = nullptr;
  };

} }  // namespace graphene::chain
