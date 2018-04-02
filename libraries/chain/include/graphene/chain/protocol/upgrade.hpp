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
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

  struct update_upgrade_event_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type upgrade_creator;
    upgrade_event_id_type upgrade_event_id;
    optional<time_point_sec> execution_time;
    optional<time_point_sec> cutoff_time;
    optional<vector<time_point_sec>> subsequent_execution_times;
    optional<string> comment;

    extensions_type extensions;

    update_upgrade_event_operation() = default;
    explicit update_upgrade_event_operation(account_id_type upgrade_creator,
                                            upgrade_event_id_type upgrade_event_id,
                                            optional<time_point_sec> execution_time,
                                            optional<time_point_sec> cutoff_time,
                                            optional<vector<time_point_sec>> subsequent_execution_times,
                                            optional<string> comment)
      : upgrade_creator(upgrade_creator),
        upgrade_event_id(upgrade_event_id),
        execution_time(execution_time),
        cutoff_time(cutoff_time),
        subsequent_execution_times(subsequent_execution_times),
        comment(comment) { }

    account_id_type fee_payer() const { return upgrade_creator; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

  struct delete_upgrade_event_operation : public base_operation
  {
    struct fee_parameters_type {};
    asset fee;   // always zero

    account_id_type upgrade_creator;
    upgrade_event_id_type upgrade_event_id;

    extensions_type extensions;

    delete_upgrade_event_operation() = default;
    explicit delete_upgrade_event_operation(account_id_type upgrade_creator, upgrade_event_id_type upgrade_event_id)
      : upgrade_creator(upgrade_creator),
        upgrade_event_id(upgrade_event_id) { }

    account_id_type fee_payer() const { return upgrade_creator; }
    void validate() const;
    share_type calculate_fee(const fee_parameters_type& k) const { return 0; }
  };

} }  // namespace graphene::chain

////////////////////////////////
// REFLECTIONS:               //
////////////////////////////////

FC_REFLECT( graphene::chain::create_upgrade_event_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::create_upgrade_event_operation,
            (fee)
            (upgrade_creator)
            (execution_time)
            (cutoff_time)
            (subsequent_execution_times)
            (comment)
            (extensions)
)

FC_REFLECT( graphene::chain::update_upgrade_event_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::update_upgrade_event_operation,
            (fee)
            (upgrade_creator)
            (upgrade_event_id)
            (execution_time)
            (cutoff_time)
            (subsequent_execution_times)
            (comment)
            (extensions)
)

FC_REFLECT( graphene::chain::delete_upgrade_event_operation::fee_parameters_type, )
FC_REFLECT( graphene::chain::delete_upgrade_event_operation,
            (fee)
            (upgrade_creator)
            (upgrade_event_id)
)
