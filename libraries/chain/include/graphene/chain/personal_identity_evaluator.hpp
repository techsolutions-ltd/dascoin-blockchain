/**
 * DASCOIN!
 */
#pragma once
#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {

class update_pi_limits_evaluator : public evaluator<update_pi_limits_evaluator>
{
public:
  typedef update_pi_limits_operation operation_type;

  void_result do_evaluate(const operation_type& op);
  void_result do_apply(const operation_type& op);

  const account_object* acnt;
};

} }  // namespace graphene::chain
