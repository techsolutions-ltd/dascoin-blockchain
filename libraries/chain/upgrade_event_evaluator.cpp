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
    const auto& gpo = d.get_global_properties();
    const auto& dgpo = d.get_dynamic_global_properties();

    d.perform_chain_authority_check("license administration", license_admin_id, op_creator_obj);

    FC_ASSERT( op.execution_time.sec_since_epoch() % gpo.parameters.maintenance_interval == 0,
               "Cannot create upgrade event whose execution time is not a multiply of maintenance interval ");

    FC_ASSERT( op.execution_time > hbt,
               "Cannot create upgrade event in the past, head block time is ${now}, execution time is ${exec}",
               ("now", hbt)
               ("exec", op.execution_time)
             );

    // Check that we don't have a scheduled event which has the same execution time:
    const auto& idx = d.get_index_type<upgrade_event_index>().indices().get<by_id>();
    for ( auto it = idx.begin(); it != idx.end(); ++it )
      FC_ASSERT( it->execution_time != op.execution_time,
                 "Cannot create upgrade event which has the same execution time as the previously created event");

    for (const auto& i : op.subsequent_execution_times)
    {
      FC_ASSERT( i.sec_since_epoch() % gpo.parameters.maintenance_interval == 0,
                 "Cannot create subsequent upgrade event whose execution time is not a multiply of maintenance interval ");
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

  void_result update_upgrade_event_evaluator::do_evaluate(const operation_type &op)
  { try {

    const auto& d = db();
    const auto& hbt = d.head_block_time();
    const upgrade_event_object& o = op.upgrade_event_id(d);
    const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
    const auto& op_creator_obj = op.upgrade_creator(d);

    d.perform_chain_authority_check("license administration", license_admin_id, op_creator_obj);

    FC_ASSERT( !o.executed, "Cannot update upgrade event which has been executed." );

    if (op.execution_time.valid())
    {
      FC_ASSERT( *op.execution_time > hbt,
                 "Cannot update upgrade event to be executed in the past, head block time is ${now}, execution time is ${exec}",
                 ("now", hbt)
                 ("exec", *op.execution_time)
      );
    }

    if (op.cutoff_time.valid())
    {
      FC_ASSERT( *op.cutoff_time > hbt,
                 "Cannot update cutoff to be in the past, head block time is ${now}, cutoff time is ${cut}",
                 ("now", hbt)
                 ("cut", *op.cutoff_time)
      );
    }

    if (op.subsequent_execution_times.valid())
    {
      for (const auto& i : *op.subsequent_execution_times)
      {
        FC_ASSERT( i > hbt,
                   "Cannot update subsequent upgrade event to be executed in the past, head block time is ${now}, event time is ${exec}",
                   ("now", hbt)
                   ("exec", i)
        );
      }
    }

    _upgrade_event = &o;

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result update_upgrade_event_evaluator::do_apply(const operation_type& op)
  { try {
    auto& d = db();
    d.modify(*_upgrade_event, [&](upgrade_event_object& ueo){
      if (op.execution_time.valid())
        ueo.execution_time = *op.execution_time;
      if (op.cutoff_time.valid())
        ueo.cutoff_time = *op.cutoff_time;
      if (op.subsequent_execution_times.valid())
        ueo.subsequent_execution_times = *op.subsequent_execution_times;
      if (op.comment.valid())
        ueo.comment = *op.comment;
    });

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result delete_upgrade_event_evaluator::do_evaluate(const operation_type &op)
  { try {

    const auto& d = db();
    const upgrade_event_object& o = op.upgrade_event_id(d);
    const auto license_admin_id = d.get_global_properties().authorities.license_administrator;
    const auto& op_creator_obj = op.upgrade_creator(d);

    d.perform_chain_authority_check("license administration", license_admin_id, op_creator_obj);

    FC_ASSERT( !o.executed, "Cannot delete upgrade event which has been executed" );

    _upgrade_event = &o;

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

  void_result delete_upgrade_event_evaluator::do_apply(const operation_type& op)
  { try {
    auto& d = db();

    d.remove(*_upgrade_event);

    return {};

  } FC_CAPTURE_AND_RETHROW( (op) ) }

} }  // namespace graphene::chain
