#include <graphene/chain/change_fee_evaluator.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/db/object_id.hpp>

namespace graphene { namespace chain {

void change_fee_evaluator::perform_root_authority_check(database& db, const account_id_type& authority_account_id)
{
   FC_ASSERT(db.get_dynamic_global_properties().is_root_authority_enabled_flag, "Your authority is deprecated!");

   const auto root_administrator_id = db.get_global_properties().authorities.root_administrator;
   const auto& op_authority_account_obj = authority_account_id(db);

   db.perform_chain_authority_check("root authority", root_administrator_id, op_authority_account_obj);
}

void_result change_fee_evaluator::do_evaluate(const change_fee_for_operation& op)
{ try {
   database& _db = db();
   perform_root_authority_check(_db, op.issuer);


} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result change_fee_evaluator::do_apply(const change_fee_for_operation& op)
{ try {

   database& _db = db();

   _db.modify(_db.get_global_properties(), [&op](global_property_object& p) {
      operation op_to_change_fee;
      op_to_change_fee.set_which(op.op_num);
      p.parameters.current_fees->change_fee(op_to_change_fee, op.new_fee);
   });

} FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
