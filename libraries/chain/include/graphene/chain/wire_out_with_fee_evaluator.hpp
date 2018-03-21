/**
 * DASCOIN!
 */
#pragma once

#include <graphene/chain/wire_out_with_fee_object.hpp>
#include <graphene/chain/evaluator.hpp>
#include <graphene/chain/protocol/wire_out_with_fee.hpp>

namespace graphene { namespace chain {

  class wire_out_with_fee_evaluator : public evaluator<wire_out_with_fee_evaluator>
  {
  public:
    typedef wire_out_with_fee_operation operation_type;

    void_result do_evaluate(const wire_out_with_fee_operation& op);
    object_id_type do_apply(const wire_out_with_fee_operation& op);

    const asset_dynamic_data_object* asset_dyn_data_ = nullptr;
    const account_balance_object*    from_balance_obj_ = nullptr;
  };

  class wire_out_with_fee_complete_evaluator : public evaluator<wire_out_with_fee_complete_evaluator>
  {
  public:
    typedef wire_out_with_fee_complete_operation operation_type;

    void_result do_evaluate(const wire_out_with_fee_complete_operation& op);
    void_result do_apply(const wire_out_with_fee_complete_operation& op);

    const wire_out_with_fee_holder_object* holder_ = nullptr;
  };

  class wire_out_with_fee_reject_evaluator : public evaluator<wire_out_with_fee_reject_evaluator>
  {
  public:
    typedef wire_out_with_fee_reject_operation operation_type;

    void_result do_evaluate(const wire_out_with_fee_reject_operation& op);
    void_result do_apply(const wire_out_with_fee_reject_operation& op);

    const asset_dynamic_data_object* asset_dyn_data_ = nullptr;
    const account_balance_object* balance_obj_ = nullptr;
    const wire_out_with_fee_holder_object* holder_ = nullptr;
  };

} }  // namespace graphene::chain
