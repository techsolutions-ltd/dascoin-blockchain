/**
 * DASCOIN!
 */
#pragma once
#include <graphene/chain/protocol/base.hpp>

namespace graphene { namespace chain {

    struct create_upgrade_event_operation : public base_operation
    {
      struct fee_parameters_type {};
      asset fee;   // always zero

      account_id_type upgrade_creator;
      time_point_sec execution_time;
      optional<time_point_sec> cutoff_time;
      vector<time_point_sec> subsequent_execution_times;
      string comment;

      extensions_type extensions;

      create_upgrade_event_operation() = default;
      explicit create_upgrade_event_operation(account_id_type upgrade_creator,
                                              time_point_sec execution_time,
                                              optional<time_point_sec> cutoff_time,
                                              vector<time_point_sec> subsequent_execution_times,
                                              string comment)
        : upgrade_creator(upgrade_creator),
          execution_time(execution_time),
          cutoff_time(cutoff_time),
          subsequent_execution_times(std::move(subsequent_execution_times)),
          comment(std::move(comment)) { }

      account_id_type fee_payer() const { return upgrade_creator; }
      void validate() const { };
      share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
    };
} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////
FC_REFLECT( graphene::chain::create_upgrade_event_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::create_upgrade_event_operation,
            (upgrade_creator)
            (execution_time)
            (cutoff_time)
            (subsequent_execution_times)
            (comment)
            (extensions)
)
