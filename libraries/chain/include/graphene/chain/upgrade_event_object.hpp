/*
 * MIT License
 *
 * Copyright (c) 2018 TechSolutions Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
      bool historic = false;
      uint16_t num_of_executions = 0;

      extensions_type extensions;

      upgrade_event_object() = default;
      explicit upgrade_event_object(time_point_sec execution_time, optional<time_point_sec> cutoff_time,
                                    vector<time_point_sec> subsequent_execution_times, string comment)
      : execution_time(execution_time),
        cutoff_time(cutoff_time),
        subsequent_execution_times(std::move(subsequent_execution_times)),
        comment(move(comment))
      {}

      bool executed() const
      {
        return num_of_executions >= subsequent_execution_times.size() + 1;
      }
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
                    (num_of_executions)
                    (extensions)
                  )
