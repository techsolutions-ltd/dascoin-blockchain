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

#include <graphene/chain/account_object.hpp>
#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/witness_object.hpp>

namespace graphene { namespace chain {

asset database::get_balance(account_id_type owner, asset_id_type asset_id) const
{
   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(owner, asset_id));
   if( itr == index.end() )
      return asset(0, asset_id);
   return itr->get_balance();
}

asset database::get_balance(const account_object& owner, const asset_object& asset_obj) const
{
   return get_balance(owner.get_id(), asset_obj.get_id());
}

asset database::get_reserved_balance(account_id_type owner, asset_id_type asset_id) const
{
   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(owner, asset_id));
   if( itr == index.end() )
      return asset(0, asset_id);
   return itr->get_reserved_balance();
}
/// This is an overloaded method.
asset database::get_reserved_balance(const account_object& owner, const asset_object& asset_obj) const
{
   return get_reserved_balance(owner.id, asset_obj.id);
}

const account_balance_object& database::get_balance_object(account_id_type owner, asset_id_type asset_id) const
{
   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(owner, asset_id));
   FC_ASSERT( itr != index.end(), "Account '${n}' has no balance object", ("n", owner(*this).name) );
   return *itr;
}

object_id_type database::create_empty_balance(account_id_type owner_id, asset_id_type asset_id)
{
   return create<account_balance_object>([&](account_balance_object& abo) {
      abo.owner = owner_id;
      abo.asset_type = asset_id;
      abo.balance = 0;
      abo.reserved = 0;
   }).id;
}

/*void database::evaluate_transfer(account_id_type from, asset delta, share_type delta_reserved, bool check_limits) const
{
   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(from, delta.asset_id));
   // Check if the object was found
   // Check the cash part of the balance >= delta
   // Check the reserved part of the balance >= delta_reserved:
   bool amount_ok = (itr != index.end() && itr->get_balance() >= delta && itr->reserved >= delta_reserved);
   FC_ASSERT( amount_ok, "Insufficient balance in account '${a}'", ("a",from(*this).name) );
   if ( check_limits )
      FC_ASSERT( itr->check_limits(delta.amount, delta_reserved), "Limit exceeded on account '${a}'",
                 ("a",from(*this).name)
               );
}

void database::complete_transfer(account_id_type from_id, account_id_type to_id, asset delta, share_type delta_reserved,
                                 bool update_spent)
{
   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto from = index.find(boost::make_tuple(from_id, delta.asset_id));
   modify(*from, [&](account_balance_object& b){
      b.balance -= delta.amount;
      b.reserved -= delta_reserved;
      if ( update_spent )
         b.increase_spent(delta.amount, delta_reserved);
   });

   auto to = index.find(boost::make_tuple(to_id, delta.asset_id));
   if ( to == index.end() )
   {
      create<account_balance_object>([&](account_balance_object& abo) {
         abo.owner = to_id;
         abo.asset_type = delta.asset_id;
         abo.balance = delta.amount.value;
         abo.reserved = delta_reserved;
      });
   } else {
      modify(*to, [&](account_balance_object& abo) {
         abo.balance += delta.amount;
         abo.reserved += delta_reserved;
      });
   }
}*/

string database::to_pretty_string( const asset& a )const
{
   return a.asset_id(*this).amount_to_pretty_string(a.amount);
}

share_type database::get_cycle_balance(account_id_type owner) const
{
   const auto& idx = get_index_type<account_cycle_balance_index>().indices().get<by_account_id>();
   const auto& itr = idx.find(owner);
   if( itr == idx.end() )
      return 0;
   return itr->balance;
}

share_type database::get_cycle_balance(const account_object& owner) const
{
   return get_cycle_balance(owner.get_id());
}

void database::adjust_balance(account_id_type account, asset delta, bool modify_limit)
{ try {
   if( delta.amount == 0 )
      return;

   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(account, delta.asset_id));
   if(itr == index.end())
   {
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                 ("a",account(*this).name)
                 ("b",to_pretty_string(asset(0,delta.asset_id)))
                 ("r",to_pretty_string(-delta)));
      create<account_balance_object>([account,&delta](account_balance_object& b) {
         b.owner = account;
         b.asset_type = delta.asset_id;
         b.balance = delta.amount.value;
      });
   } else {
      if( delta.amount < 0 )
         FC_ASSERT( itr->get_balance() >= -delta,
                    "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                    ("a",account(*this).name)
                    ("b",to_pretty_string(itr->get_balance()))
                    ("r",to_pretty_string(-delta))
                  );
      modify(*itr, [delta, modify_limit](account_balance_object& b) {
         b.adjust_balance(delta);
         // We adjust the limit ONLY IF the asset is REMOVED from the balance so the delta must be negative!
         if ( modify_limit && delta.amount < 0 )
            b.spent -= delta.amount;
      });
   }

} FC_CAPTURE_AND_RETHROW( (account)(delta) ) }

void database::adjust_cycle_balance(account_id_type account, share_type delta, optional<uint8_t> upgrades)
{ try {

   if( delta == 0 )
      return;

   if ( upgrades.valid() )
      FC_ASSERT( *upgrades > 0, "Upgrades must be increased by a positive amont: upgrades = ${u} ", ("u",upgrades) );

   auto& index = get_index_type<account_cycle_balance_index>().indices().get<by_account_id>();
   auto itr = index.find(account);
   if(itr == index.end())
   {
      FC_ASSERT( delta > 0, "Insufficient Balance: ${a}'s balance of cycles is less than required ${r}",
                 ("a",account(*this).name)
                 ("r",to_pretty_string(-delta)));

      create<account_cycle_balance_object>([account,delta,upgrades](account_cycle_balance_object& b) {
         b.owner = account;
         b.balance = delta;
         if ( upgrades.valid() )
            b.remaining_upgrades = *upgrades;
      });
   } else {
      if( delta < 0 )
         FC_ASSERT( itr->get_balance() >= -delta,
                    "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                    ("a",account(*this).name)
                    ("b",to_pretty_string(itr->get_balance()))
                    ("r",to_pretty_string(-delta)));

      modify(*itr, [delta,upgrades](account_cycle_balance_object& b) {
         b.adjust_cycle_balance(delta);
         if ( upgrades.valid() )
            b.adjust_upgrades(*upgrades);
      });
   }

} FC_CAPTURE_AND_RETHROW( (account)(delta) ) }

optional<limits_type> database::get_account_limits(const account_id_type account) const
{ try {
   auto& index = get_index_type<account_index>().indices().get<by_id>();
   auto itr = index.find(account);
   if (itr != index.end())
      return {itr->limits};
   return {};
} FC_CAPTURE_AND_RETHROW( (account) ) }

optional<uint8_t> database::get_account_pi_level(const account_id_type account) const
{ try {
   auto& index = get_index_type<account_index>().indices().get<by_id>();
   auto itr = index.find(account);
   if (itr != index.end())
      return {itr->pi_level};
   return {};
} FC_CAPTURE_AND_RETHROW( (account) ) }

optional< vesting_balance_id_type > database::deposit_lazy_vesting(
   const optional< vesting_balance_id_type >& ovbid,
   share_type amount, uint32_t req_vesting_seconds,
   account_id_type req_owner,
   bool require_vesting )
{
   if( amount == 0 )
      return optional< vesting_balance_id_type >();

   fc::time_point_sec now = head_block_time();

   while( true )
   {
      if( !ovbid.valid() )
         break;
      const vesting_balance_object& vbo = (*ovbid)(*this);
      if( vbo.owner != req_owner )
         break;
      if( vbo.policy.which() != vesting_policy::tag< cdd_vesting_policy >::value )
         break;
      if( vbo.policy.get< cdd_vesting_policy >().vesting_seconds != req_vesting_seconds )
         break;
      modify( vbo, [&]( vesting_balance_object& _vbo )
      {
         if( require_vesting )
            _vbo.deposit(now, amount);
         else
            _vbo.deposit_vested(now, amount);
      } );
      return optional< vesting_balance_id_type >();
   }

   const vesting_balance_object& vbo = create< vesting_balance_object >( [&]( vesting_balance_object& _vbo )
   {
      _vbo.owner = req_owner;
      _vbo.balance = amount;

      cdd_vesting_policy policy;
      policy.vesting_seconds = req_vesting_seconds;
      policy.coin_seconds_earned = require_vesting ? 0 : amount.value * policy.vesting_seconds;
      policy.coin_seconds_earned_last_update = now;

      _vbo.policy = policy;
   } );

   return vbo.id;
}

void database::deposit_cashback(const account_object& acct, share_type amount, bool require_vesting)
{
   // If we don't have a VBO, or if it has the wrong maturity
   // due to a policy change, cut it loose.

   if( amount == 0 )
      return;

   if( acct.get_id() == GRAPHENE_COMMITTEE_ACCOUNT || acct.get_id() == GRAPHENE_WITNESS_ACCOUNT ||
       acct.get_id() == GRAPHENE_RELAXED_COMMITTEE_ACCOUNT || acct.get_id() == GRAPHENE_NULL_ACCOUNT ||
       acct.get_id() == GRAPHENE_TEMP_ACCOUNT )
   {
      // The blockchain's accounts do not get cashback; it simply goes to the reserve pool.
      modify(get(asset_id_type()).dynamic_asset_data_id(*this), [amount](asset_dynamic_data_object& d) {
         d.current_supply -= amount;
      });
      return;
   }

   optional< vesting_balance_id_type > new_vbid = deposit_lazy_vesting(
      acct.cashback_vb,
      amount,
      get_global_properties().parameters.cashback_vesting_period_seconds,
      acct.id,
      require_vesting );

   if( new_vbid.valid() )
   {
      modify( acct, [&]( account_object& _acct )
      {
         _acct.cashback_vb = *new_vbid;
      } );
   }

   return;
}

void database::deposit_witness_pay(const witness_object& wit, share_type amount)
{
   if( amount == 0 )
      return;

   optional< vesting_balance_id_type > new_vbid = deposit_lazy_vesting(
      wit.pay_vb,
      amount,
      get_global_properties().parameters.witness_pay_vesting_seconds,
      wit.witness_account,
      true );

   if( new_vbid.valid() )
   {
      modify( wit, [&]( witness_object& _wit )
      {
         _wit.pay_vb = *new_vbid;
      } );
   }

   return;
}

} }
