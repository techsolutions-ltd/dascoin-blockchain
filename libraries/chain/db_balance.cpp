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

const account_balance_object& database::get_balance_object(account_id_type owner, asset_id_type asset_id) const
{
   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(owner, asset_id));
   FC_ASSERT( itr != index.end(), "Account '${n}' has no balance object for ${a}",
              ("n", owner(*this).name)
              ("a", asset_id(*this).symbol)
            );
   return *itr;
}

const account_cycle_balance_object& database::get_cycle_balance_object(account_id_type owner) const
{
  auto& index = get_index_type<account_cycle_balance_index>().indices().get<by_account_id>();
  auto itr = index.find(owner);
  FC_ASSERT( itr != index.end(), "Account '${n}' has no cycle balance object", ("n", owner(*this).name) );
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

object_id_type database::create_empty_cycle_balance(account_id_type owner_id)
{
   return create<account_cycle_balance_object>([&](account_cycle_balance_object& acbo) {
      acbo.owner = owner_id;
      acbo.balance = 0;
   }).id;
}

string database::to_pretty_string(const asset& a) const
{
   return a.asset_id(*this).amount_to_pretty_string(a.amount);
}

string database::to_pretty_string(const asset_reserved& a) const
{
   return a.asset_id(*this).amount_to_pretty_string(a);
}

string database::to_pretty_string(const account_balance_object& abo) const
{
  return abo.asset_type(*this).amount_to_pretty_string(abo.get_asset_reserved_balance());
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

void database::issue_cycles(account_id_type account, share_type amount)
{ try {

  issue_cycles(get_cycle_balance_object(account), amount);

} FC_CAPTURE_AND_RETHROW( (account)(amount) ) }

void database::issue_cycles(const account_cycle_balance_object& balance, share_type amount)
{
  FC_ASSERT( amount > 0, "Attempting to issue ${am} cycles, the value must be greater than zero", ("am", amount) );

  modify(balance, [amount](account_cycle_balance_object& b) {
     b.balance += amount;
  });

  modify(get_dynamic_global_properties(), [amount](dynamic_global_property_object& dgpo){
    dgpo.cycle_supply += amount;
    dgpo.total_cycles_issued += amount;
  });

}

void database::reserve_cycles(account_id_type account, share_type amount)
{ try {

  reserve_cycles(get_cycle_balance_object(account), amount);

} FC_CAPTURE_AND_RETHROW( (account)(amount) ) }

void database::reserve_cycles(const account_cycle_balance_object& balance, share_type amount)
{
  FC_ASSERT( amount > 0, "Attempting to reserve ${am} cycles, the value must be greater than zero", ("am", amount) );

  modify(balance, [amount](account_cycle_balance_object& b) {
     b.balance -= amount;
  });

  modify(get_dynamic_global_properties(), [amount](dynamic_global_property_object& dgpo){
    dgpo.cycle_supply -= amount;
  });
}

void database::issue_asset(account_id_type account_id, share_type cash, asset_id_type asset_id, share_type reserved)
{ try {

   issue_asset(get_balance_object(account_id, asset_id), cash, reserved);

} FC_CAPTURE_AND_RETHROW((account_id)(asset_id)(cash)(reserved)) }

void database::issue_asset(const account_balance_object& balance_obj, share_type cash, share_type reserved)
{ try {

   if ( cash == 0 && reserved == 0 ) // allow issuing of reserved balance only
     return;

   modify(balance_obj, [cash, reserved](account_balance_object& b) {
      b.balance += cash;
      b.reserved += reserved;
   });

   const auto& asset_obj = balance_obj.asset_type(*this);
   modify(asset_obj.dynamic_asset_data_id(*this), [&](asset_dynamic_data_object& data){
      data.current_supply += cash;
      data.current_supply += reserved;
   });

} FC_CAPTURE_AND_RETHROW((cash)(reserved)) }

void database::adjust_balance(account_id_type account, asset delta, share_type reserved_delta)
{ try {
   if( delta.amount == 0 && reserved_delta == 0 ) // allow adjusting of reserved balance only
      return;

   auto& index = get_index_type<account_balance_index>().indices().get<by_account_asset>();
   auto itr = index.find(boost::make_tuple(account, delta.asset_id));
   if(itr == index.end())
   {
      // bool amounts_ok = delta.amount > 0 && reserved_delta > 0;
      FC_ASSERT( delta.amount > 0, "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                 ("a",account(*this).name)
                 ("b",to_pretty_string(asset(0,delta.asset_id)))
                 ("r",to_pretty_string(-delta)));
      // NOTE: the reserved amount CAN BE ZERO!
      FC_ASSERT( reserved_delta >= 0, "Insufficient Balance: ${a}'s reserved balance of ${b} is less than required ${r}",
                 ("a",account(*this).name)
                 ("b",to_pretty_string(asset(0,delta.asset_id)))
                 ("r",to_pretty_string(-asset(reserved_delta, delta.asset_id))));
      create<account_balance_object>([account, &delta, reserved_delta](account_balance_object& b) {
         b.owner = account;
         b.asset_type = delta.asset_id;
         b.balance = delta.amount.value;
         b.reserved = reserved_delta;
      });
   } else {
      if( delta.amount < 0 )
         FC_ASSERT( itr->get_balance() >= -delta,
                    "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                    ("a",account(*this).name)
                    ("b",to_pretty_string(itr->get_balance()))
                    ("r",to_pretty_string(-delta))
                  );
      if ( reserved_delta < 0)
         FC_ASSERT( itr->reserved >= -reserved_delta,
                    "Insufficient Balance: ${a}'s balance of ${b} is less than required ${r}",
                    ("a",account(*this).name)
                    ("b",to_pretty_string(itr->get_reserved_balance()))
                    ("r",to_pretty_string(asset(-reserved_delta, delta.asset_id)))
                  );
      modify(*itr, [delta, reserved_delta](account_balance_object& b) {
         b.adjust_balance(delta);
         b.reserved += reserved_delta;
      });
   }

} FC_CAPTURE_AND_RETHROW( (account)(delta) ) }

void database::adjust_cycle_balance(account_id_type account, share_type delta)
{ try {

   if( delta == 0 )
      return;

   auto& index = get_index_type<account_cycle_balance_index>().indices().get<by_account_id>();
   auto itr = index.find(account);

   FC_ASSERT( itr != index.end(), "Account '${n}' has no cycle balance object", ("n", account(*this).name) );

   if( delta < 0 )
      FC_ASSERT( itr->get_balance() >= -delta,
                 "Insufficient Cycle Balance: ${a}'s balance of ${b} is less than required ${r}",
                 ("a",account(*this).name)
                 ("b", itr->get_balance())
                 ("r", -delta)
               );

   modify(*itr, [delta](account_cycle_balance_object& b) {
      b.balance += delta;
   });

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
