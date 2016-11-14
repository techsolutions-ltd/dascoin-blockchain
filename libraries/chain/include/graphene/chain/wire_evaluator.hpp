/**
 * DASCOIN!
 */
#pragma once

#include <graphene/chain/wire_object.hpp>
#include <graphene/chain/evaluator.hpp>

namespace graphene { namespace chain {

  class wire_out_evaluator : public evaluator<wire_out_evaluator>
  {
  public:
    typedef wire_out_operation operation_type;

    void_result do_evaluate(const wire_out_operation& op);

    object_id_type do_apply(const wire_out_operation& op);
  };

} }  // namespace graphene::chain
