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

#include <graphene/chain/database.hpp>
#include <graphene/chain/db_with.hpp>

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/global_property_object.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/license_objects.hpp>
#include <graphene/chain/market_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/queue_objects.hpp>
#include <graphene/chain/transaction_object.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/witness_object.hpp>

#include <graphene/chain/protocol/fee_schedule.hpp>

#include <fc/uint128.hpp>

namespace graphene { namespace chain {

void database::update_global_dynamic_data( const signed_block& b )
{
   const dynamic_global_property_object& _dgp =
      dynamic_global_property_id_type(0)(*this);

   uint32_t missed_blocks = get_slot_at_time( b.timestamp );
   assert( missed_blocks != 0 );
   missed_blocks--;
   for( uint32_t i = 0; i < missed_blocks; ++i ) {
      const auto& witness_missed = get_scheduled_witness( i+1 )(*this);
      if(  witness_missed.id != b.witness ) {
         /*
         const auto& witness_account = witness_missed.witness_account(*this);
         if( (fc::time_point::now() - b.timestamp) < fc::seconds(30) )
            wlog( "Witness ${name} missed block ${n} around ${t}", ("name",witness_account.name)("n",b.block_num())("t",b.timestamp) );
            */

         modify( witness_missed, [&]( witness_object& w ) {
           w.total_missed++;
         });
      }
   }

   // dynamic global properties updating
   modify( _dgp, [&]( dynamic_global_property_object& dgp ){
      if( BOOST_UNLIKELY( b.block_num() == 1 ) )
         dgp.recently_missed_count = 0;
         else if( _checkpoints.size() && _checkpoints.rbegin()->first >= b.block_num() )
         dgp.recently_missed_count = 0;
      else if( missed_blocks )
         dgp.recently_missed_count += GRAPHENE_RECENTLY_MISSED_COUNT_INCREMENT*missed_blocks;
      else if( dgp.recently_missed_count > GRAPHENE_RECENTLY_MISSED_COUNT_INCREMENT )
         dgp.recently_missed_count -= GRAPHENE_RECENTLY_MISSED_COUNT_DECREMENT;
      else if( dgp.recently_missed_count > 0 )
         dgp.recently_missed_count--;

      dgp.head_block_number = b.block_num();
      dgp.head_block_id = b.id();
      dgp.time = b.timestamp;
      dgp.current_witness = b.witness;
      dgp.recent_slots_filled = (
           (dgp.recent_slots_filled << 1)
           + 1) << missed_blocks;
      dgp.current_aslot += missed_blocks+1;
   });

   if( !(get_node_properties().skip_flags & skip_undo_history_check) )
   {
      GRAPHENE_ASSERT( _dgp.head_block_number - _dgp.last_irreversible_block_num  < GRAPHENE_MAX_UNDO_HISTORY, undo_database_exception,
                 "The database does not have enough undo history to support a blockchain with so many missed blocks. "
                 "Please add a checkpoint if you would like to continue applying blocks beyond this point.",
                 ("last_irreversible_block_num",_dgp.last_irreversible_block_num)("head", _dgp.head_block_number)
                 ("recently_missed",_dgp.recently_missed_count)("max_undo",GRAPHENE_MAX_UNDO_HISTORY) );
   }

   _undo_db.set_max_size( _dgp.head_block_number - _dgp.last_irreversible_block_num + 1 );
   _fork_db.set_max_size( _dgp.head_block_number - _dgp.last_irreversible_block_num + 1 );
}

void database::update_signing_witness(const witness_object& signing_witness, const signed_block& new_block)
{
   const global_property_object& gpo = get_global_properties();
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();
   uint64_t new_block_aslot = dpo.current_aslot + get_slot_at_time( new_block.timestamp );

   share_type witness_pay = std::min( gpo.parameters.witness_pay_per_block, dpo.witness_budget );

   modify( dpo, [&]( dynamic_global_property_object& _dpo )
   {
      _dpo.witness_budget -= witness_pay;
   } );

   deposit_witness_pay( signing_witness, witness_pay );

   modify( signing_witness, [&]( witness_object& _wit )
   {
      _wit.last_aslot = new_block_aslot;
      _wit.last_confirmed_block_num = new_block.block_num();
   } );
}

void database::update_last_irreversible_block()
{
   const global_property_object& gpo = get_global_properties();
   const dynamic_global_property_object& dpo = get_dynamic_global_properties();

   vector< const witness_object* > wit_objs;
   wit_objs.reserve( gpo.active_witnesses.size() );
   for( const witness_id_type& wid : gpo.active_witnesses )
      wit_objs.push_back( &(wid(*this)) );

   static_assert( GRAPHENE_IRREVERSIBLE_THRESHOLD > 0, "irreversible threshold must be nonzero" );

   // 1 1 1 2 2 2 2 2 2 2 -> 2     .7*10 = 7
   // 1 1 1 1 1 1 1 2 2 2 -> 1
   // 3 3 3 3 3 3 3 3 3 3 -> 3

   size_t offset = ((GRAPHENE_100_PERCENT - GRAPHENE_IRREVERSIBLE_THRESHOLD) * wit_objs.size() / GRAPHENE_100_PERCENT);

   std::nth_element( wit_objs.begin(), wit_objs.begin() + offset, wit_objs.end(),
      []( const witness_object* a, const witness_object* b )
      {
         return a->last_confirmed_block_num < b->last_confirmed_block_num;
      } );

   uint32_t new_last_irreversible_block_num = wit_objs[offset]->last_confirmed_block_num;

   if( new_last_irreversible_block_num > dpo.last_irreversible_block_num )
   {
      modify( dpo, [&]( dynamic_global_property_object& _dpo )
      {
         _dpo.last_irreversible_block_num = new_last_irreversible_block_num;
      } );
   }
}

void database::clear_expired_transactions()
{ try {
   //Look for expired transactions in the deduplication list, and remove them.
   //Transactions must have expired by at least two forking windows in order to be removed.
   auto& transaction_idx = static_cast<transaction_index&>(get_mutable_index(implementation_ids, impl_transaction_object_type));
   const auto& dedupe_index = transaction_idx.indices().get<by_expiration>();
   while( (!dedupe_index.empty()) && (head_block_time() > dedupe_index.rbegin()->trx.expiration) )
      transaction_idx.remove(*dedupe_index.rbegin());
} FC_CAPTURE_AND_RETHROW() }

void database::clear_expired_proposals()
{
   const auto& proposal_expiration_index = get_index_type<proposal_index>().indices().get<by_expiration>();
   while( !proposal_expiration_index.empty() && proposal_expiration_index.begin()->expiration_time <= head_block_time() )
   {
      const proposal_object& proposal = *proposal_expiration_index.begin();
      processed_transaction result;
      try {
         if( proposal.is_authorized_to_execute(*this) )
         {
            result = push_proposal(proposal);
            //TODO: Do something with result so plugins can process it.
            continue;
         }
      } catch( const fc::exception& e ) {
         elog("Failed to apply proposed transaction on its expiration. Deleting it.\n${proposal}\n${error}",
              ("proposal", proposal)("error", e.to_detail_string()));
      }
      remove(proposal);
   }
}

/**
 *  let HB = the highest bid for the collateral  (aka who will pay the most DEBT for the least collateral)
 *  let SP = current median feed's Settlement Price
 *  let LC = the least collateralized call order's swan price (debt/collateral)
 *
 *  If there is no valid price feed or no bids then there is no black swan.
 *
 *  A black swan occurs if MAX(HB,SP) <= LC
 */
bool database::check_for_blackswan( const asset_object& mia, bool enable_black_swan )
{
    if( !mia.is_market_issued() ) return false;

    const asset_bitasset_data_object& bitasset = mia.bitasset_data(*this);
    if( bitasset.has_settlement() ) return true; // already force settled
    auto settle_price = bitasset.current_feed.settlement_price;
    if( settle_price.is_null() ) return false; // no feed

    const call_order_index& call_index = get_index_type<call_order_index>();
    const auto& call_price_index = call_index.indices().get<by_price>();

    const limit_order_index& limit_index = get_index_type<limit_order_index>();
    const auto& limit_price_index = limit_index.indices().get<by_price>();

    // looking for limit orders selling the most USD for the least CORE
    auto highest_possible_bid = price::max( mia.id, bitasset.options.short_backing_asset );
    // stop when limit orders are selling too little USD for too much CORE
    auto lowest_possible_bid  = price::min( mia.id, bitasset.options.short_backing_asset );

    assert( highest_possible_bid.base.asset_id == lowest_possible_bid.base.asset_id );
    // NOTE limit_price_index is sorted from greatest to least
    auto limit_itr = limit_price_index.lower_bound( highest_possible_bid );
    auto limit_end = limit_price_index.upper_bound( lowest_possible_bid );

    auto call_min = price::min( bitasset.options.short_backing_asset, mia.id );
    auto call_max = price::max( bitasset.options.short_backing_asset, mia.id );
    auto call_itr = call_price_index.lower_bound( call_min );
    auto call_end = call_price_index.upper_bound( call_max );

    if( call_itr == call_end ) return false;  // no call orders

    price highest = settle_price;
    if( limit_itr != limit_end ) {
       assert( settle_price.base.asset_id == limit_itr->sell_price.base.asset_id );
       highest = std::max( limit_itr->sell_price, settle_price );
    }

    auto least_collateral = call_itr->collateralization();
    if( ~least_collateral >= highest  )
    {
       elog( "Black Swan detected: \n"
             "   Least collateralized call: ${lc}  ${~lc}\n"
           //  "   Highest Bid:               ${hb}  ${~hb}\n"
             "   Settle Price:              ${sp}  ${~sp}\n"
             "   Max:                       ${h}   ${~h}\n",
            ("lc",least_collateral.to_real())("~lc",(~least_collateral).to_real())
          //  ("hb",limit_itr->sell_price.to_real())("~hb",(~limit_itr->sell_price).to_real())
            ("sp",settle_price.to_real())("~sp",(~settle_price).to_real())
            ("h",highest.to_real())("~h",(~highest).to_real()) );
       FC_ASSERT( enable_black_swan, "Black swan was detected during a margin update which is not allowed to trigger a blackswan" );
       globally_settle_asset(mia, ~least_collateral );
       return true;
    }
    return false;
}

void database::clear_expired_orders()
{ try {
   bool send_updates = false;
   detail::with_skip_flags( *this,
      get_node_properties().skip_flags | skip_authority_check, [&](){
         transaction_evaluation_state cancel_context(this);

         //Cancel expired limit orders
         auto& limit_index = get_index_type<limit_order_index>().indices().get<by_expiration>();
         while( !limit_index.empty() && limit_index.begin()->expiration <= head_block_time() )
         {
            limit_order_cancel_operation canceler;
            const limit_order_object& order = *limit_index.begin();
            canceler.fee_paying_account = order.seller;
            canceler.order = order.id;
            canceler.fee = current_fee_schedule().calculate_fee( canceler );
            if( canceler.fee.amount > order.deferred_fee )
            {
               // Cap auto-cancel fees at deferred_fee; see #549
               wlog( "At block ${b}, fee for clearing expired order ${oid} was capped at deferred_fee ${fee}", ("b", head_block_num())("oid", order.id)("fee", order.deferred_fee) );
               canceler.fee = asset( order.deferred_fee, asset_id_type() );
            }
            // we know the fee for this op is set correctly since it is set by the chain.
            // this allows us to avoid a hung chain:
            // - if #549 case above triggers
            // - if the fee is incorrect, which may happen due to #435 (although since cancel is a fixed-fee op, it shouldn't)
            cancel_context.skip_fee_schedule_check = true;
            apply_operation(cancel_context, canceler);
            send_updates = true;
         }
     });

   if (send_updates)
       notify_changed_objects();

   // TODO: call notify_changed_objects when handling settlements too.
   //Process expired force settlement orders
   auto& settlement_index = get_index_type<force_settlement_index>().indices().get<by_expiration>();
   if( !settlement_index.empty() )
   {
      asset_id_type current_asset = settlement_index.begin()->settlement_asset_id();
      asset max_settlement_volume;
      bool extra_dump = false;

      auto next_asset = [&current_asset, &settlement_index, &extra_dump] {
         auto bound = settlement_index.upper_bound(current_asset);
         if( bound == settlement_index.end() )
         {
            if( extra_dump )
            {
               ilog( "next_asset() returning false" );
            }
            return false;
         }
         if( extra_dump )
         {
            ilog( "next_asset returning true, bound is ${b}", ("b", *bound) );
         }
         current_asset = bound->settlement_asset_id();
         return true;
      };

      uint32_t count = 0;

      // At each iteration, we either consume the current order and remove it, or we move to the next asset
      for( auto itr = settlement_index.lower_bound(current_asset);
           itr != settlement_index.end();
           itr = settlement_index.lower_bound(current_asset) )
      {
         ++count;
         const force_settlement_object& order = *itr;
         auto order_id = order.id;
         current_asset = order.settlement_asset_id();
         const asset_object& mia_object = get(current_asset);
         const asset_bitasset_data_object& mia = mia_object.bitasset_data(*this);

         extra_dump = ((count >= 1000) && (count <= 1020));

         if( extra_dump )
         {
            wlog( "clear_expired_orders() dumping extra data for iteration ${c}", ("c", count) );
            ilog( "head_block_num is ${hb} current_asset is ${a}", ("hb", head_block_num())("a", current_asset) );
         }

         if( mia.has_settlement() )
         {
            ilog( "Canceling a force settlement because of black swan" );
            cancel_order( order );
            continue;
         }

         // Has this order not reached its settlement date?
         if( order.settlement_date > head_block_time() )
         {
            if( next_asset() )
            {
               if( extra_dump )
               {
                  ilog( "next_asset() returned true when order.settlement_date > head_block_time()" );
               }
               continue;
            }
            break;
         }
         // Can we still settle in this asset?
         if( mia.current_feed.settlement_price.is_null() )
         {
            ilog("Canceling a force settlement in ${asset} because settlement price is null",
                 ("asset", mia_object.symbol));
            cancel_order(order);
            continue;
         }
         if( max_settlement_volume.asset_id != current_asset )
            max_settlement_volume = mia_object.amount(mia.max_force_settlement_volume(mia_object.dynamic_data(*this).current_supply));
         if( mia.force_settled_volume >= max_settlement_volume.amount )
         {
            /*
            ilog("Skipping force settlement in ${asset}; settled ${settled_volume} / ${max_volume}",
                 ("asset", mia_object.symbol)("settlement_price_null",mia.current_feed.settlement_price.is_null())
                 ("settled_volume", mia.force_settled_volume)("max_volume", max_settlement_volume));
                 */
            if( next_asset() )
            {
               if( extra_dump )
               {
                  ilog( "next_asset() returned true when mia.force_settled_volume >= max_settlement_volume.amount" );
               }
               continue;
            }
            break;
         }

         auto& pays = order.balance;
         auto receives = (order.balance * mia.current_feed.settlement_price);
         receives.amount = (fc::uint128_t(receives.amount.value) *
                            (GRAPHENE_100_PERCENT - mia.options.force_settlement_offset_percent) / GRAPHENE_100_PERCENT).to_uint64();
         assert(receives <= order.balance * mia.current_feed.settlement_price);

         price settlement_price = pays / receives;

         auto& call_index = get_index_type<call_order_index>().indices().get<by_collateral>();
         asset settled = mia_object.amount(mia.force_settled_volume);
         // Match against the least collateralized short until the settlement is finished or we reach max settlements
         while( settled < max_settlement_volume && find_object(order_id) )
         {
            auto itr = call_index.lower_bound(boost::make_tuple(price::min(mia_object.bitasset_data(*this).options.short_backing_asset,
                                                                           mia_object.get_id())));
            // There should always be a call order, since asset exists!
            assert(itr != call_index.end() && itr->debt_type() == mia_object.get_id());
            asset max_settlement = max_settlement_volume - settled;

            if( order.balance.amount == 0 )
            {
               wlog( "0 settlement detected" );
               cancel_order( order );
               break;
            }
            try {
               settled += match(*itr, order, settlement_price, max_settlement);
            }
            catch ( const black_swan_exception& e ) {
               wlog( "black swan detected: ${e}", ("e", e.to_detail_string() ) );
               cancel_order( order );
               break;
            }
         }
         if( mia.force_settled_volume != settled.amount )
         {
            modify(mia, [settled](asset_bitasset_data_object& b) {
               b.force_settled_volume = settled.amount;
            });
         }
      }
   }
} FC_CAPTURE_AND_RETHROW() }

void database::update_expired_feeds()
{
   auto& asset_idx = get_index_type<asset_index>().indices().get<by_type>();
   auto itr = asset_idx.lower_bound( true /** market issued */ );
   while( itr != asset_idx.end() )
   {
      const asset_object& a = *itr;
      ++itr;
      assert( a.is_market_issued() );

      const asset_bitasset_data_object& b = a.bitasset_data(*this);
      bool feed_is_expired;
      if( head_block_time() < HARDFORK_615_TIME )
         feed_is_expired = b.feed_is_expired_before_hardfork_615( head_block_time() );
      else
         feed_is_expired = b.feed_is_expired( head_block_time() );
      if( feed_is_expired )
      {
         modify(b, [this](asset_bitasset_data_object& a) {
            a.update_median_feeds(head_block_time());
         });
         check_call_orders(b.current_feed.settlement_price.base.asset_id(*this));
      }
      if( !b.current_feed.core_exchange_rate.is_null() &&
          a.options.core_exchange_rate != b.current_feed.core_exchange_rate )
         modify(a, [&b](asset_object& a) {
            a.options.core_exchange_rate = b.current_feed.core_exchange_rate;
         });
   }
}

void database::update_maintenance_flag( bool new_maintenance_flag )
{
   modify( get_dynamic_global_properties(), [&]( dynamic_global_property_object& dpo )
   {
      auto maintenance_flag = dynamic_global_property_object::maintenance_flag;
      dpo.dynamic_flags =
           (dpo.dynamic_flags & ~maintenance_flag)
         | (new_maintenance_flag ? maintenance_flag : 0);
   } );
   return;
}

void database::update_withdraw_permissions()
{
   auto& permit_index = get_index_type<withdraw_permission_index>().indices().get<by_expiration>();
   while( !permit_index.empty() && permit_index.begin()->expiration <= head_block_time() )
      remove(*permit_index.begin());
}

void database::distribute_issue_requested_assets()
{ try {
  transaction_evaluation_state distribute_context(this);
  const auto& idx = get_index_type<issue_asset_request_index>().indices().get<by_expiration>();

  while (!idx.empty() && idx.begin()->expiration <= head_block_time())
  {
    const auto& req = *idx.begin();

    issue_asset(req.receiver, req.amount, req.asset_id, req.reserved_amount);

    asset_distribute_completed_request_operation vop;
    vop.issuer = req.issuer;
    vop.receiver = req.receiver;
    vop.amount = req.amount;
    push_applied_operation(vop);

    remove(req);
  }
} FC_CAPTURE_AND_RETHROW() }

void database::reset_spending_limits()
{ try {
  const auto& params = get_global_properties().parameters;
  const auto& dgpo = get_dynamic_global_properties();

  if ( dgpo.next_spend_limit_reset <= head_block_time() )
  {
    // Reset spending limit for each account:
    const auto& account_idx = get_index_type<account_index>().indices().get<by_id>();
    for ( const auto& account : account_idx )
    {
      // TODO: price should be a weekly average price, not the last price at the moment of sampling.
      auto dsc_limit = get_dascoin_limit(account, dgpo.last_dascoin_price);
      if ( dsc_limit.valid() )
      {
        // Set the limit on the account balance object:
        adjust_balance_limit(account, get_dascoin_asset_id(), *dsc_limit, true);
      }
    }

    // Set the time of the next limit reset:
    modify(dgpo, [&](dynamic_global_property_object& dgpo){
      dgpo.last_daily_dascoin_price = dgpo.last_dascoin_price;
      uint32_t now_sec = head_block_time().sec_since_epoch();
      uint32_t next_interval = (now_sec / params.limit_interval_elapse_time_seconds) *
                                params.limit_interval_elapse_time_seconds + params.limit_interval_elapse_time_seconds;
      // Move to the next interval if the current calculation returns the same point as the previous one:
      if (fc::time_point_sec(dgpo.next_spend_limit_reset) == fc::time_point_sec(next_interval))
        dgpo.next_spend_limit_reset = fc::time_point_sec(next_interval) + params.limit_interval_elapse_time_seconds;
      else
        dgpo.next_spend_limit_reset = fc::time_point_sec(next_interval);
    });
  }

} FC_CAPTURE_AND_RETHROW() }

void database::mint_dascoin_rewards()
{ try {
  const auto& params = get_global_properties().parameters;
  const auto& dgpo = get_dynamic_global_properties();
  auto last_minted_number = dgpo.last_minted_submission_num;

  if ( dgpo.next_dascoin_reward_time <= head_block_time() )
  {
    auto to_distribute = get_global_properties().parameters.dascoin_reward_amount;
    share_type total_distributed = 0;

    const auto& queue = get_index_type<reward_queue_index>().indices().get<by_time>();
    while ( to_distribute > 0 && !queue.empty() )
    {
      const auto& el = *queue.begin();
      auto dascoin_amount = cycles_to_dascoin(el.amount, el.frequency);
      if ( to_distribute >= dascoin_amount )
      {
        // TODO: refactor this call?
        issue_asset(el.account, dascoin_amount, get_dascoin_asset_id(), 0);
        // Emit a virtual operation:
        push_applied_operation(record_distribute_dascoin_operation(el.origin, el.license, el.account,
                                                                   el.amount, el.frequency, 
                                                                   dascoin_amount, head_block_time()));
        remove(el);
        last_minted_number++;
      }
      else
      {
        dascoin_amount = to_distribute;
        // TODO: refactor this call?
        issue_asset(el.account, dascoin_amount, get_dascoin_asset_id(), 0);
        // Emit a virtual operation:
        push_applied_operation(record_distribute_dascoin_operation(el.origin, el.license, el.account,
                                                                   el.amount, el.frequency, 
                                                                   dascoin_amount, head_block_time()));
        share_type cycles = dascoin_to_cycles(dascoin_amount, el.frequency);
        modify(el, [cycles](reward_queue_object& rqo){
          rqo.amount -= cycles;
        });
      }

      total_distributed += dascoin_amount;
      to_distribute -= dascoin_amount;
    }

    modify(dgpo, [&](dynamic_global_property_object& dgpo){
      dgpo.next_dascoin_reward_time = head_block_time() + params.reward_interval_time_seconds;
      dgpo.total_dascoin_minted += total_distributed;
      dgpo.last_minted_submission_num = last_minted_number;
    });
  }

} FC_CAPTURE_AND_RETHROW() }

void database::daspay_clearing_start()
{ try {
  const auto& params = get_global_properties();
  const auto& dgpo = get_dynamic_global_properties();

  if ( dgpo.daspay_next_clearing_time > head_block_time() )
    return;

  const auto& idx = get_index_type<payment_service_provider_index>().indices().get<by_payment_service_provider>();
  flat_set<account_id_type> clearing_accounts;
  for (auto it = idx.cbegin(); it != idx.cend(); ++it)
  {
    std::copy(it->payment_service_provider_clearing_accounts.begin(), it->payment_service_provider_clearing_accounts.end(), std::inserter(clearing_accounts, clearing_accounts.end()));
  }

  if (clearing_accounts.empty())
    return;

  const auto& apply_tx = [this, &params](const vector<limit_order_create_operation>& ops) {
    bool was_undo_db_enabled = _undo_db.enabled();
    if (!was_undo_db_enabled)
      _undo_db.enable();
    signed_transaction trx;
    trx.set_reference_block(head_block_id());
    trx.set_expiration( head_block_time() + fc::seconds( params.parameters.block_interval * (params.parameters.maintenance_skip_slots + 1) * 3 ) );

    std::copy(ops.begin(), ops.end(), std::back_inserter(trx.operations));

    const fee_schedule& current_fees = get_global_properties().parameters.current_fees;
    for( auto& op : trx.operations )
      current_fees.set_fee(op);

    apply_transaction(trx, ~0);
    if (!was_undo_db_enabled)
      _undo_db.disable();
  };

  vector<limit_order_create_operation> limit_orders;
  flat_set<share_type> sell_prices;
  flat_set<share_type> buy_prices;
  const auto& das_id = get_dascoin_asset_id();
  const auto& web_id = get_web_asset_id();
  get_groups_of_limit_order_prices(das_id, web_id, sell_prices, true, 2);
  get_groups_of_limit_order_prices(web_id, das_id, buy_prices, false, 2);

  for (const auto& clearing_acc : clearing_accounts)
  {
    const fee_schedule& current_fees = get_global_properties().parameters.current_fees;
    const auto& fee = current_fees.calculate_fee(limit_order_create_operation());
    const auto& fee_balance = get_balance(clearing_acc, fee.asset_id);
    if (fee > fee_balance)
    {
      wlog("Clearing account ${a} has insufficient balance to pay fee (Balance: ${b}, Fee: ${c})", ("a", clearing_acc)("b", to_pretty_string(fee_balance))("c", to_pretty_string(fee)));
      continue;
    }

    const auto& dasc_balance = get_balance(clearing_acc, get_dascoin_asset_id());
    const auto& webeur_balance = get_balance(clearing_acc, get_web_asset_id());

    if (dasc_balance.amount > params.daspay_parameters.collateral_dascoin)
    {
      // If there are no buy orders, do nothing
      if (buy_prices.empty())
        continue;

      auto to_sell = dasc_balance - asset{ params.daspay_parameters.collateral_dascoin, get_dascoin_asset_id() };
      auto price_it = buy_prices.begin();
      if (head_block_time() < HARDFORK_BLC_217_TIME)
      {
        // Before HARDFORK_BLC_217_TIME this was invalid, prices are sorted in ascending order, so the second best
        // price is actually the first one
        std::advance(price_it, buy_prices.size() - 1); // Use the second price if available

        // we cannot spend the last dasc
        if (params.daspay_parameters.collateral_dascoin < 1 * DASCOIN_DEFAULT_ASSET_PRECISION)
          to_sell -= asset{ 1 * DASCOIN_DEFAULT_ASSET_PRECISION, get_dascoin_asset_id() };
      }
      share_type buy_price = *price_it;

      const auto& to_buy = asset{ to_sell.amount * buy_price / DASCOIN_DEFAULT_ASSET_PRECISION / 1000, get_web_asset_id() };

      if (to_sell.amount > 0 && to_buy.amount > 0)
        limit_orders.emplace_back(limit_order_create_operation{ clearing_acc, to_sell, to_buy, 0, {}, head_block_time() + params.daspay_parameters.clearing_interval_time_seconds });
    }
    else if (webeur_balance.amount >= params.daspay_parameters.collateral_webeur && dasc_balance.amount < params.daspay_parameters.collateral_dascoin)
    {
      // If there are no sell orders, do nothing
      if (sell_prices.empty())
        continue;

      const auto& to_buy = asset{ params.daspay_parameters.collateral_dascoin, get_dascoin_asset_id() } - dasc_balance;
      auto price_it = sell_prices.begin();
      std::advance(price_it, sell_prices.size() - 1); // Use the second price if available
      share_type buy_price = *price_it;

      const auto& to_sell = asset{ to_buy.amount * buy_price / DASCOIN_DEFAULT_ASSET_PRECISION / 1000, get_web_asset_id() };
      if (webeur_balance >= to_sell && to_sell.amount > 0 && to_buy.amount > 0)
        limit_orders.emplace_back(limit_order_create_operation{ clearing_acc, to_sell, to_buy, 0, {}, head_block_time() + params.daspay_parameters.clearing_interval_time_seconds });
      else
        wlog("Clearing account ${a} has insufficient balance ${b}", ("a", clearing_acc)("b", to_pretty_string(webeur_balance)));
    }
  }

  if (!limit_orders.empty())
  { try {
    apply_tx(limit_orders);
  } FC_CAPTURE_AND_LOG( (limit_orders) ) }

  modify(dgpo, [&](dynamic_global_property_object& dgpo){
    dgpo.daspay_next_clearing_time = head_block_time() + params.daspay_parameters.clearing_interval_time_seconds;
  });

} FC_CAPTURE_AND_RETHROW() }

void database::resolve_delayed_operations()
{ try {
  const auto& params = get_global_properties();
  const auto& dgpo = get_dynamic_global_properties();

  if ( dgpo.next_delayed_operations_resolver_time > head_block_time() )
    return;

  const auto& idx = get_index_type<delayed_operations_index>().indices().get<by_account>();
  for (auto it = idx.cbegin(); it != idx.cend(); ++it)
  {
    if (it->issued_time + it->skip <= head_block_time())
    {
      it->op.visit(op_visitor(*this));
      remove(*it);
    }
  }

  modify(dgpo, [&](dynamic_global_property_object& dgpo){
    dgpo.next_delayed_operations_resolver_time = head_block_time() + params.delayed_operations_resolver_interval_time_seconds;
  });

} FC_CAPTURE_AND_RETHROW() }

} }  // namespace database::chain
