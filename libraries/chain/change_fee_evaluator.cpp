#include <graphene/chain/change_fee_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/db/object_id.hpp>

namespace graphene { namespace chain {

void_result change_fee_evaluator::do_evaluate(const change_fee_for_operation& op)
{ try {
   database& d = db();
   d.perform_root_authority_check(op.issuer);


} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result change_fee_evaluator::do_apply(const change_fee_for_operation& op)
{ try {

   database& d = db();

   d.modify(d.get_global_properties(), [&op](global_property_object& p) {
      operation op_to_change_fee;
      op_to_change_fee.set_which(op.op_num);
      p.parameters.current_fees->change_fee(op_to_change_fee, op.new_fee);
   });

} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result change_fee_pool_account_evaluator::do_evaluate(const change_fee_pool_account_operation& op)
{ try {
   database& d = db();
   d.perform_root_authority_check(op.issuer);


} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result change_fee_pool_account_evaluator::do_apply(const change_fee_pool_account_operation& op)
{ try {

   database& d = db();

   d.modify(d.get_dynamic_global_properties(), [&op](dynamic_global_property_object& p) {
      p.fee_pool_account_id = op.fee_pool_account_id;

   });

} FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
