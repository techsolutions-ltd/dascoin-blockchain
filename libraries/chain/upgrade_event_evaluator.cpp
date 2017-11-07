/**
 * DASCOIN!
 */
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/upgrade_event_evaluator.hpp>

namespace graphene { namespace chain {

  void_result create_upgrade_event_evaluator::do_evaluate(const operation_type &op)
  { try {

    const auto& d = db();
    const auto& hbt = d.head_block_time();
    const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
    const auto& op_creator_obj = op.upgrade_creator(d);

    d.perform_chain_authority_check("license administration", license_admin_id, op_creator_obj);

    FC_ASSERT( op.execution_time > hbt,
               "Cannot create upgrade event in the past, head block time is ${now}, execution time is ${exec}",
               ("now", hbt)
               ("exec", op.execution_time)
             );

    if (op.cutoff_time.valid())
    {
      FC_ASSERT( *op.cutoff_time > hbt,
                 "Cannot create cutoff in the past, head block time is ${now}, cutoff time is ${cut}",
                 ("now", hbt)
                 ("cut", *op.cutoff_time)
               );
    }

    for (const auto& i : op.subsequent_execution_times)
    {
      FC_ASSERT( i > hbt,
                 "Cannot create subsequent upgrade event in the past, head block time is ${now}, event time is ${exec}",
                 ("now", hbt)
                 ("exec", i)
               );
    }

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  object_id_type create_upgrade_event_evaluator::do_apply(const operation_type& op)
  { try {
    auto& d = db();
    return d.create<upgrade_event_object>([&](upgrade_event_object& ueo){
      ueo.execution_time = op.execution_time;
      ueo.cutoff_time = op.cutoff_time;
      ueo.subsequent_execution_times = op.subsequent_execution_times;
      ueo.comment = op.comment;
    }).id;

  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
