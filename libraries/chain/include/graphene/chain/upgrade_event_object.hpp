/**
 * DASCOIN!
 */

#pragma once
#include <graphene/db/object.hpp>

namespace graphene { namespace chain {

  ///////////////////////////////
  // OBJECTS:                  //
  ///////////////////////////////
  /**
   * @class upgrade_event_object
   * @brief Upgrade event which is scheduled to be run sometime in the future.
   * @ingroup object
   *
   * Contains information to schedule an upgrade event
   */

  class upgrade_event_object : public abstract_object<upgrade_event_object>
  {
    public:
      static const uint8_t space_id = protocol_ids;
      static const uint8_t type_id  = upgrade_event_object_type;

      time_point_sec execution_time;
      optional<time_point_sec> cutoff_time;
      vector<time_point_sec> subsequent_execution_times;
      string comment;

      extensions_type extensions;

      upgrade_event_object() = default;
      explicit upgrade_event_object(time_point_sec execution_time, optional<time_point_sec> cutoff_time,
                                    vector<time_point_sec> subsequent_execution_times, string comment)
      : execution_time(execution_time),
        cutoff_time(cutoff_time),
        subsequent_execution_times(std::move(subsequent_execution_times)),
        comment(move(comment))
      {}
  };

  ///////////////////////////////
  // MULTI INDEX CONTAINERS:   //
  ///////////////////////////////
  struct by_execution_time;
  using upgrade_event_multi_index_type = multi_index_container<
    upgrade_event_object,
    indexed_by<
      ordered_unique<
        tag<by_id>,
        member< object, object_id_type, &object::id >
      >,
      ordered_non_unique<
        tag<by_execution_time>,
          composite_key< upgrade_event_object,
            member< upgrade_event_object, time_point_sec, &upgrade_event_object::execution_time >,
            member< object, object_id_type, &object::id >
          >
      >
    >
  >;

  using upgrade_event_index = generic_index<upgrade_event_object, upgrade_event_multi_index_type>;

} }  // namespace graphene::chain

///////////////////////////////
// REFLECTIONS:              //
///////////////////////////////

FC_REFLECT_DERIVED( graphene::chain::upgrade_event_object, (graphene::db::object),
                    (execution_time)
                    (cutoff_time)
                    (subsequent_execution_times)
                    (comment)
                    (extensions)
                  )
