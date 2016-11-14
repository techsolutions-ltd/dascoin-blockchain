/**
 * DASCOIN!
 */
#include <graphene/chain/wire_evaluator.hpp>

namespace graphene { namespace chain {

  void_result wire_out_evaluator::do_evaluate(const wire_out_operation& op)
  { try {

    const auto& _db = db();

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  object_id_type wire_out_evaluator::do_apply(const wire_out_operation& op)
  { try {
    return {};
  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
