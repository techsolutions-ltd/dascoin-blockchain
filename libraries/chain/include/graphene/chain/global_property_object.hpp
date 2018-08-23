/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <fc/uint128.hpp>

#include <graphene/chain/protocol/chain_parameters.hpp>
#include <graphene/chain/protocol/chain_authorities.hpp>
#include <graphene/chain/protocol/types.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/db/object.hpp>

namespace graphene { namespace chain {

   /**
    * @class global_property_object
    * @brief Maintains global state information (committee_member list, current fees)
    * @ingroup object
    * @ingroup implementation
    *
    * This is an implementation detail. The values here are set by committee_members to tune the blockchain parameters.
    */
   class global_property_object : public graphene::db::abstract_object<global_property_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_global_property_object_type;

         chain_parameters           parameters;
         optional<chain_parameters> pending_parameters;

         chain_authorities          authorities;

         uint32_t                           next_available_vote_id = 0;
         vector<committee_member_id_type>   active_committee_members; // updated once per maintenance interval
         flat_set<witness_id_type>          active_witnesses; // updated once per maintenance interval
         // n.b. witness scheduling is done by witness_schedule object

         struct daspay
         {
           bool clearing_enabled = DASPAY_DEFAULT_CLEARING_ENABLED; ///< by default off
           uint32_t clearing_interval_time_seconds = DASPAY_DEFAULT_CLEARING_INTERVAL_TIME_SECONDS; ///< in seconds
           share_type collateral_dascoin = DASPAY_DEFAULT_CLEARING_COLLATERAL_DASC; ///< by default set to 0
           share_type collateral_webeur = DASPAY_DEFAULT_CLEARING_COLLATERAL_WEBEUR; ///< by default set to 0
         };
         daspay daspay_parameters;

         bool delayed_operations_resolver_enabled = DASCOIN_DEFAULT_DELAYED_OPERATIONS_RESOLVER_ENABLED; ///< by default off
         uint32_t delayed_operations_resolver_interval_time_seconds = DASCOIN_DEFAULT_DELAYED_OPERATIONS_RESOLVER_INTERVAL_TIME_SECONDS; ///< in seconds

   };

   /**
    * @class dynamic_global_property_object
    * @brief Maintains global state information (committee_member list, current fees)
    * @ingroup object
    * @ingroup implementation
    *
    * This is an implementation detail. The values here are calculated during normal chain operations and reflect the
    * current values of global blockchain properties.
    */
   class dynamic_global_property_object : public abstract_object<dynamic_global_property_object>
   {
      public:
         static const uint8_t space_id = implementation_ids;
         static const uint8_t type_id  = impl_dynamic_global_property_object_type;

         uint32_t          head_block_number = 0;
         block_id_type     head_block_id;
         time_point_sec    time;
         witness_id_type   current_witness;
         time_point_sec    next_maintenance_time;
         time_point_sec    last_budget_time;
         share_type        witness_budget;
         uint32_t          accounts_registered_this_interval = 0;
         /**
          *  Every time a block is missed this increases by
          *  RECENTLY_MISSED_COUNT_INCREMENT,
          *  every time a block is found it decreases by
          *  RECENTLY_MISSED_COUNT_DECREMENT.  It is
          *  never less than 0.
          *
          *  If the recently_missed_count hits 2*UNDO_HISTORY then no new blocks may be pushed.
          */
         uint32_t          recently_missed_count = 0;

         /**
          * The current absolute slot number.  Equal to the total
          * number of slots since genesis.  Also equal to the total
          * number of missed slots plus head_block_number.
          */
         uint64_t                current_aslot = 0;

         /**
          * used to compute witness participation.
          */
         fc::uint128_t recent_slots_filled;

         /**
          * Current supply of cycles in all balances.
          */
         share_type cycle_supply = 0;

         /**
          * Total amount of cycles issued since genesis.
          */
         share_type total_cycles_issued = 0;

         /**
          * Total Dascoin produced by the minting process.
          */
         share_type total_dascoin_minted = 0;

         /**
          * Last minted submission number.
          */
         uint64_t last_minted_submission_num = 0;

         /**
          * The largest submission number in the queue.
          */
         uint64_t max_queue_submission_num = 0;

         /**
          * The current global frequency:
          */
         frequency_type frequency;

         /**
          * DasPay
          */
         time_point_sec daspay_next_clearing_time = fc::time_point_sec();
         share_type daspay_debit_transaction_ratio = 0;
         share_type daspay_credit_transaction_ratio = 0;

         /**
          * Delayed operations resolver
          */
         time_point_sec next_delayed_operations_resolver_time = fc::time_point_sec();

         /**
          * dynamic_flags specifies chain state properties that can be
          * expressed in one bit.
          */
         uint32_t dynamic_flags = 0;

         uint32_t last_irreversible_block_num = 0;

         /**
          * The next time Dascoin will be minted and distributed.
          */
         time_point_sec next_dascoin_reward_time = fc::time_point_sec();

         /**
          * The time point the spend limit will be reset for all balances.
          * NOTE: the value is set to maximum at the beginning to trigger a reset on the first block.
          */
         time_point_sec next_spend_limit_reset = fc::time_point_sec();

         /**
          * Last dascoin trade price on the DSC:WEBEUR market.
          */
         price last_dascoin_price;

         /**
          * Last bitcoin trade price on the BTC:WEBEUR market
          */
         price external_btc_price;

         /**
          * Last daily dascoin trade price on the DSC:WEBEUR market.
          */
         price last_daily_dascoin_price;

         /**
          * This flag is used for enabling use of root authority
          * which is used for granting operation for adding multiple master nodes.
          */
         bool is_root_authority_enabled_flag = true;

         /**
          * This is id of account that owns fee pool balance object
          * It can be set or empy if empty cycles will be burned when fee is payed
          */
         account_id_type fee_pool_account_id;

         enum dynamic_flag_bits
         {
            /**
             * If maintenance_flag is set, then the head block is a
             * maintenance block.  This means
             * get_time_slot(1) - head_block_time() will have a gap
             * due to maintenance duration.
             *
             * This flag answers the question, "Was maintenance
             * performed in the last call to apply_block()?"
             */
            maintenance_flag = 0x01,
            upgrade_flag = 0x02,
         };
   };
}}

FC_REFLECT_DERIVED( graphene::chain::dynamic_global_property_object, (graphene::db::object),
                    (head_block_number)
                    (head_block_id)
                    (time)
                    (current_witness)
                    (next_maintenance_time)
                    (last_budget_time)
                    (witness_budget)
                    (accounts_registered_this_interval)
                    (recently_missed_count)
                    (current_aslot)
                    (recent_slots_filled)
                    (cycle_supply)
                    (total_cycles_issued)
                    (total_dascoin_minted)
                    (last_minted_submission_num)
                    (max_queue_submission_num)
                    (frequency)
                    (daspay_debit_transaction_ratio)
                    (daspay_credit_transaction_ratio)
                    (daspay_next_clearing_time)
                    (next_delayed_operations_resolver_time)
                    (dynamic_flags)
                    (last_irreversible_block_num)
                    (next_dascoin_reward_time)
                    (next_spend_limit_reset)
                    (is_root_authority_enabled_flag)
                    (last_dascoin_price)
                    (external_btc_price)
                    (fee_pool_account_id)
                  )

FC_REFLECT( graphene::chain::global_property_object::daspay,
            (clearing_enabled)
            (clearing_interval_time_seconds)
            (collateral_dascoin)
            (collateral_webeur)
          )

FC_REFLECT_DERIVED( graphene::chain::global_property_object, (graphene::db::object),
                    (parameters)
                    (pending_parameters)
                    (next_available_vote_id)
                    (active_committee_members)
                    (authorities)
                    (active_witnesses)
                    (daspay_parameters)
                    (delayed_operations_resolver_enabled)
                    (delayed_operations_resolver_interval_time_seconds)
                  )
