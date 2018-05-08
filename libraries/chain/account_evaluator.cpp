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

#include <fc/smart_ref_impl.hpp>

#include <graphene/chain/account_evaluator.hpp>
#include <graphene/chain/buyback.hpp>
#include <graphene/chain/buyback_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/internal_exceptions.hpp>
#include <graphene/chain/special_authority.hpp>
#include <graphene/chain/special_authority_object.hpp>
#include <graphene/chain/worker_object.hpp>
#include <algorithm>

namespace graphene { namespace chain {

void verify_authority_accounts( const database& db, const authority& a )
{
   const auto& chain_params = db.get_global_properties().parameters;
   GRAPHENE_ASSERT(
      a.num_auths() <= chain_params.maximum_authority_membership,
      internal_verify_auth_max_auth_exceeded,
      "Maximum authority membership exceeded" );
   for( const auto& acnt : a.account_auths )
   {
      GRAPHENE_ASSERT( db.find_object( acnt.first ) != nullptr,
         internal_verify_auth_account_not_found,
         "Account ${a} specified in authority does not exist",
         ("a", acnt.first) );
   }
}

void verify_account_votes( const database& db, const account_options& options )
{
   // ensure account's votes satisfy requirements
   // NB only the part of vote checking that requires chain state is here,
   // the rest occurs in account_options::validate()

   const auto& gpo = db.get_global_properties();
   const auto& chain_params = gpo.parameters;

   FC_ASSERT( options.num_witness <= chain_params.maximum_witness_count,
              "Voted for more witnesses than currently allowed (${c})", ("c", chain_params.maximum_witness_count) );
   FC_ASSERT( options.num_committee <= chain_params.maximum_committee_count,
              "Voted for more committee members than currently allowed (${c})", ("c", chain_params.maximum_committee_count) );

   uint32_t max_vote_id = gpo.next_available_vote_id;
   bool has_worker_votes = false;
   for( auto id : options.votes )
   {
      FC_ASSERT( id < max_vote_id );
      has_worker_votes |= (id.type() == vote_id_type::worker);
   }

   if( has_worker_votes && (db.head_block_time() >= HARDFORK_607_TIME) )
   {
      const auto& against_worker_idx = db.get_index_type<worker_index>().indices().get<by_vote_against>();
      for( auto id : options.votes )
      {
         if( id.type() == vote_id_type::worker )
         {
            FC_ASSERT( against_worker_idx.find( id ) == against_worker_idx.end() );
         }
      }
   }

}


void_result account_create_evaluator::do_evaluate( const account_create_operation& op )
{ try {
   database& d = db();
   account_id_type registrar_id = d.get_global_properties().authorities.registrar;

   if( d.head_block_time() < HARDFORK_516_TIME )
   {
      FC_ASSERT( !op.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !op.extensions.value.active_special_authority.valid() );
   }
   if( d.head_block_time() < HARDFORK_599_TIME )
   {
      FC_ASSERT( !op.extensions.value.null_ext.valid() );
      FC_ASSERT( !op.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !op.extensions.value.active_special_authority.valid() );
      FC_ASSERT( !op.extensions.value.buyback_options.valid() );
   }

   FC_ASSERT( d.find_object(op.options.voting_account), "Invalid proxy account specified." );
   FC_ASSERT( op.referrer(d).is_member(d.head_block_time()), "The referrer must be either a lifetime or annual subscriber." );

   // Check for validity of chain authorities:
   if ( !skip_chain_authority_check() )
      FC_ASSERT( fee_paying_account->id == registrar_id, "Account can only be registered by current registrar chain authority" );

   try
   {
      verify_authority_accounts( d, op.owner );
      verify_authority_accounts( d, op.active );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_create_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_create_auth_account_not_found )

   if( op.extensions.value.owner_special_authority.valid() )
      evaluate_special_authority( d, *op.extensions.value.owner_special_authority );
   if( op.extensions.value.active_special_authority.valid() )
      evaluate_special_authority( d, *op.extensions.value.active_special_authority );
   if( op.extensions.value.buyback_options.valid() )
      evaluate_buyback_account_options( d, *op.extensions.value.buyback_options );
   verify_account_votes( d, op.options );

   auto& acnt_indx = d.get_index_type<account_index>();
   if( op.name.size() )
   {
      auto current_account_itr = acnt_indx.indices().get<by_name>().find( op.name );
      FC_ASSERT( current_account_itr == acnt_indx.indices().get<by_name>().end() );
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type account_create_evaluator::do_apply( const account_create_operation& o )
{ try {

   database& d = db();
   uint16_t referrer_percent = o.referrer_percent;
   bool has_small_percent = (
         (db().head_block_time() <= HARDFORK_453_TIME)
      && (o.referrer != o.registrar  )
      && (o.referrer_percent != 0    )
      && (o.referrer_percent <= 0x100)
      );

   if( has_small_percent )
   {
      if( referrer_percent >= 100 )
      {
         wlog( "between 100% and 0x100%:  ${o}", ("o", o) );
      }
      referrer_percent = referrer_percent*100;
      if( referrer_percent > GRAPHENE_100_PERCENT )
         referrer_percent = GRAPHENE_100_PERCENT;
   }

   const auto& new_acnt_object = db().create<account_object>( [&]( account_object& obj ){
         obj.kind = static_cast<account_kind>(o.kind);
         obj.registrar = o.registrar;
         obj.referrer = o.referrer;
         obj.lifetime_referrer = o.referrer(db()).lifetime_referrer;

         auto& params = db().get_global_properties().parameters;
         obj.network_fee_percentage = params.network_percent_of_fee;
         obj.lifetime_referrer_fee_percentage = params.lifetime_referrer_percent_of_fee;
         obj.referrer_rewards_percentage = referrer_percent;

         obj.name             = o.name;
         obj.owner            = o.owner;
         obj.active           = o.active;
         obj.options          = o.options;
         obj.statistics = db().create<account_statistics_object>([&](account_statistics_object& s){s.owner = obj.id;}).id;

         if( o.extensions.value.owner_special_authority.valid() )
            obj.owner_special_authority = *(o.extensions.value.owner_special_authority);
         if( o.extensions.value.active_special_authority.valid() )
            obj.active_special_authority = *(o.extensions.value.active_special_authority);
         if( o.extensions.value.buyback_options.valid() )
         {
            obj.allowed_assets = o.extensions.value.buyback_options->markets;
            obj.allowed_assets->emplace( o.extensions.value.buyback_options->asset_to_buy );
         }

         // set account rollback data
        obj.owner_roll_back = obj.owner;
        obj.active_roll_back = obj.active;
        obj.roll_back_active = false;
        obj.roll_back_enabled = true;

         // from this moment there are no vault limits
         if(db().head_block_time() >= HARDFORK_EXEX_102_TIME)
            obj.disable_vault_to_wallet_limit = true;
   });

   if( has_small_percent )
   {
      wlog( "Account affected by #453 registered in block ${n}:  ${na} reg=${reg} ref=${ref}:${refp} ltr=${ltr}:${ltrp}",
         ("n", db().head_block_num()) ("na", new_acnt_object.id)
         ("reg", o.registrar) ("ref", o.referrer) ("ltr", new_acnt_object.lifetime_referrer)
         ("refp", new_acnt_object.referrer_rewards_percentage) ("ltrp", new_acnt_object.lifetime_referrer_fee_percentage) );
      wlog( "Affected account object is ${o}", ("o", new_acnt_object) );
   }

   const auto& dynamic_properties = db().get_dynamic_global_properties();
   db().modify(dynamic_properties, [](dynamic_global_property_object& p) {
      ++p.accounts_registered_this_interval;
   });

   // NOTE: fees are disabled, pay scaling is disabled as well
   // const auto& global_properties = db().get_global_properties();
   // if( dynamic_properties.accounts_registered_this_interval %
   //     global_properties.parameters.accounts_per_fee_scale == 0 )
   //    db().modify(global_properties, [&dynamic_properties](global_property_object& p) {
   //       p.parameters.current_fees->get<account_create_operation>().basic_fee <<= p.parameters.account_fee_scale_bitshifts;
   //    });

   if(    o.extensions.value.owner_special_authority.valid()
       || o.extensions.value.active_special_authority.valid() )
   {
      db().create< special_authority_object >( [&]( special_authority_object& sa )
      {
         sa.account = new_acnt_object.id;
      } );
   }

   if( o.extensions.value.buyback_options.valid() )
   {
      asset_id_type asset_to_buy = o.extensions.value.buyback_options->asset_to_buy;

      d.create< buyback_object >( [&]( buyback_object& bo )
      {
         bo.asset_to_buy = asset_to_buy;
      } );

      d.modify( asset_to_buy(d), [&]( asset_object& a )
      {
         a.buyback_account = new_acnt_object.id;
      } );
   }

   // Each account is more or less guaranteed to have some cycles assigned to it.
   db().create_empty_cycle_balance(new_acnt_object.id);

   // Same goes for dascoin and web asset(s):
   // TODO: this needs to be done of every kind of web asset there is!
   db().create_empty_balance(new_acnt_object.id, db().get_dascoin_asset_id());
   db().create_empty_balance(new_acnt_object.id, db().get_web_asset_id());
   db().create_empty_balance(new_acnt_object.id, db().get_cycle_asset_id());

   if ( new_acnt_object.is_vault() )
   {
      const auto daily_price = dynamic_properties.last_daily_dascoin_price;
      const auto starting_limit = *(db().get_dascoin_limit(new_acnt_object, daily_price));
      const auto dsc_id = db().get_dascoin_asset_id();
      db().adjust_balance_limit(new_acnt_object, dsc_id, starting_limit);
   }

   // wallets and custodians should have cycle asset with starting balance
   if (new_acnt_object.is_wallet() || new_acnt_object.is_custodian())
   {
     const share_type ZERO_RESERVED_AMOUNT = 0;
     const auto& global_props = db().get_global_properties();
     int starting_cycle_asset = global_props.parameters.starting_cycle_asset_amount;
     const auto& balance_object = db().get_balance_object(new_acnt_object.id, db().get_cycle_asset_id());
     db().issue_asset(balance_object, starting_cycle_asset, ZERO_RESERVED_AMOUNT);
   }

   return new_acnt_object.id;

} FC_CAPTURE_AND_RETHROW((o)) }


void_result account_update_evaluator::do_evaluate( const account_update_operation& o )
{ try {
   database& d = db();
   if( d.head_block_time() < HARDFORK_516_TIME )
   {
      FC_ASSERT( !o.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !o.extensions.value.active_special_authority.valid() );
   }
   if( d.head_block_time() < HARDFORK_599_TIME )
   {
      FC_ASSERT( !o.extensions.value.null_ext.valid() );
      FC_ASSERT( !o.extensions.value.owner_special_authority.valid() );
      FC_ASSERT( !o.extensions.value.active_special_authority.valid() );
   }

   try
   {
      if( o.owner )  verify_authority_accounts( d, *o.owner );
      if( o.active ) verify_authority_accounts( d, *o.active );
   }
   GRAPHENE_RECODE_EXC( internal_verify_auth_max_auth_exceeded, account_update_max_auth_exceeded )
   GRAPHENE_RECODE_EXC( internal_verify_auth_account_not_found, account_update_auth_account_not_found )

   if( o.extensions.value.owner_special_authority.valid() )
      evaluate_special_authority( d, *o.extensions.value.owner_special_authority );
   if( o.extensions.value.active_special_authority.valid() )
      evaluate_special_authority( d, *o.extensions.value.active_special_authority );

   acnt = &o.account(d);

   if( o.new_options.valid() )
      verify_account_votes( d, *o.new_options );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_update_evaluator::do_apply( const account_update_operation& o )
{ try {
   database& d = db();
   bool sa_before, sa_after;
   d.modify( *acnt, [&](account_object& a){
      if( o.owner )
      {
         a.owner = *o.owner;
         a.top_n_control_flags = 0;
      }
      if( o.active )
      {
         a.active = *o.active;
         a.top_n_control_flags = 0;
      }
      if( o.new_options ) a.options = *o.new_options;
      sa_before = a.has_special_authority();
      if( o.extensions.value.owner_special_authority.valid() )
      {
         a.owner_special_authority = *(o.extensions.value.owner_special_authority);
         a.top_n_control_flags = 0;
      }
      if( o.extensions.value.active_special_authority.valid() )
      {
         a.active_special_authority = *(o.extensions.value.active_special_authority);
         a.top_n_control_flags = 0;
      }
      sa_after = a.has_special_authority();
   });

   if( sa_before & (!sa_after) )
   {
      const auto& sa_idx = d.get_index_type< special_authority_index >().indices().get<by_account>();
      auto sa_it = sa_idx.find( o.account );
      assert( sa_it != sa_idx.end() );
      d.remove( *sa_it );
   }
   else if( (!sa_before) & sa_after )
   {
      d.create< special_authority_object >( [&]( special_authority_object& sa )
      {
         sa.account = o.account;
      } );
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_evaluate(const account_whitelist_operation& o)
{ try {
   database& d = db();

   listed_account = &o.account_to_list(d);
   if( !d.get_global_properties().parameters.allow_non_member_whitelists )
      FC_ASSERT(o.authorizing_account(d).is_lifetime_member());

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_whitelist_evaluator::do_apply(const account_whitelist_operation& o)
{ try {
   database& d = db();

   d.modify(*listed_account, [&o](account_object& a) {
      if( o.new_listing & o.white_listed )
         a.whitelisting_accounts.insert(o.authorizing_account);
      else
         a.whitelisting_accounts.erase(o.authorizing_account);

      if( o.new_listing & o.black_listed )
         a.blacklisting_accounts.insert(o.authorizing_account);
      else
         a.blacklisting_accounts.erase(o.authorizing_account);
   });

   /** for tracking purposes only, this state is not needed to evaluate */
   d.modify( o.authorizing_account(d), [&]( account_object& a ) {
     if( o.new_listing & o.white_listed )
        a.whitelisted_accounts.insert( o.account_to_list );
     else
        a.whitelisted_accounts.erase( o.account_to_list );

     if( o.new_listing & o.black_listed )
        a.blacklisted_accounts.insert( o.account_to_list );
     else
        a.blacklisted_accounts.erase( o.account_to_list );
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result account_upgrade_evaluator::do_evaluate(const account_upgrade_evaluator::operation_type& o)
{ try {
   database& d = db();

   account = &d.get(o.account_to_upgrade);
   FC_ASSERT(!account->is_lifetime_member());

   return {};
//} FC_CAPTURE_AND_RETHROW( (o) ) }
} FC_RETHROW_EXCEPTIONS( error, "Unable to upgrade account '${a}'", ("a",o.account_to_upgrade(db()).name) ) }

void_result account_upgrade_evaluator::do_apply(const account_upgrade_evaluator::operation_type& o)
{ try {
   database& d = db();

   d.modify(*account, [&](account_object& a) {
      if( o.upgrade_to_lifetime_member )
      {
         // Upgrade to lifetime member. I don't care what the account was before.
         a.statistics(d).process_fees(a, d);
         a.membership_expiration_date = time_point_sec::maximum();
         a.referrer = a.registrar = a.lifetime_referrer = a.get_id();
         a.lifetime_referrer_fee_percentage = GRAPHENE_100_PERCENT - a.network_fee_percentage;
      } else if( a.is_annual_member(d.head_block_time()) ) {
         // Renew an annual subscription that's still in effect.
         FC_ASSERT( d.head_block_time() <= HARDFORK_613_TIME );
         FC_ASSERT(a.membership_expiration_date - d.head_block_time() < fc::days(3650),
                   "May not extend annual membership more than a decade into the future.");
         a.membership_expiration_date += fc::days(365);
      } else {
         // Upgrade from basic account.
         FC_ASSERT( d.head_block_time() <= HARDFORK_613_TIME );
         a.statistics(d).process_fees(a, d);
         assert(a.is_basic_account(d.head_block_time()));
         a.referrer = a.get_id();
         a.membership_expiration_date = d.head_block_time() + fc::days(365);
      }
   });

   return {};
} FC_RETHROW_EXCEPTIONS( error, "Unable to upgrade account '${a}'", ("a",o.account_to_upgrade(db()).name) ) }

void_result tether_accounts_evaluator::do_evaluate(const tether_accounts_operation& op)
{
   auto& d = db();
   wallet_account = &d.get(op.wallet_account);
   vault_account = &d.get(op.vault_account);

   GRAPHENE_ASSERT(
      wallet_account->is_wallet(),
      tether_accounts_no_wallet_account,
      "Unable to tether account '${va}' to '${wa}', '${wa}' is not a wallet account",
      ("wa", wallet_account->name)
      ("va", vault_account->name)
   );
   GRAPHENE_ASSERT(
      vault_account->is_vault(),
      tether_accounts_no_vault_account,
      "Unable to tether account '${va}' to '${wa}', '${va}' is not a vault account",
      ("wa", wallet_account->name)
      ("va", vault_account->name)

   );
   return void_result();
}

void_result tether_accounts_evaluator::do_apply(const tether_accounts_operation& op)
{ try {
   auto& d = db();

   d.modify(*wallet_account, [&](account_object& wa) {
      wa.vault.insert(vault_account->id);
   });
   d.modify(*vault_account, [&](account_object& va) {
      va.parents.insert(wallet_account->id);
      va.hierarchy_depth = 1;
   });

   return void_result();

} FC_CAPTURE_AND_RETHROW((op)) }

void_result change_public_keys_evaluator::do_evaluate(const change_public_keys_operation& op)
{ try {
   const auto& d = db();
   _account_obj = &op.account(d);
   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type change_public_keys_evaluator::do_apply(const change_public_keys_operation& op)
{ try {

   db().modify( *_account_obj, [&](account_object& ao){
      if(op.owner)
      {
         ao.owner = *op.owner;
         ao.owner_change_counter++;
         ao.top_n_control_flags = 0;  // Legacy bitshares flag.
      }
      if(op.active)
      {
         ao.active = *op.active;
         ao.active_change_counter++;
         ao.top_n_control_flags = 0;  // Legacy bitshares flag.
      }

      // Re-enable other operations in case roll_back_active flag is set to true
      ao.roll_back_active = false;
   });

   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result toggle_roll_back_enabled_evaluator::do_evaluate(const toggle_roll_back_enabled_operation& op)
{
  try {
    return {};
  } FC_CAPTURE_AND_RETHROW((op))
}

object_id_type toggle_roll_back_enabled_evaluator::do_apply(const toggle_roll_back_enabled_operation& op)
{
  try {
    db().modify(op.account(db()), [&](account_object& ao) {ao.roll_back_enabled = !ao.roll_back_enabled;});
    return {};
  } FC_CAPTURE_AND_RETHROW((op))
}

void_result roll_back_public_keys_evaluator::do_evaluate(const roll_back_public_keys_operation& op)
{
  try {
    const auto op_authority_obj = op.authority(db());
    db().perform_chain_authority_check("personal identity validation",
                                       db().get_global_properties().authorities.pi_validator,
                                       op_authority_obj);
    FC_ASSERT(op.account(db()).roll_back_enabled, "Cannot initiate rollback procedure because account has roll_back_enabled set to false.");
    return {};
  } FC_CAPTURE_AND_RETHROW((op))
}

object_id_type roll_back_public_keys_evaluator::do_apply(const roll_back_public_keys_operation& op)
{
  try {
    db().modify(op.account(db()), [&](account_object& ao)
    {
      ao.roll_back_active = true;
      ao.owner = ao.owner_roll_back;
      ao.active = ao.active_roll_back;
    });
    return {};
  } FC_CAPTURE_AND_RETHROW((op))
}

void_result set_starting_cycle_asset_amount_evaluator::do_evaluate(const set_starting_cycle_asset_amount_operation& op)
{
  try
  {
    database& d = db();
    d.perform_root_authority_check(op.issuer);
    return {};
  } FC_CAPTURE_AND_RETHROW((op))
}

void_result set_starting_cycle_asset_amount_evaluator::do_apply(const set_starting_cycle_asset_amount_operation& op)
{
  try
  {
    auto& d = db();
    const auto& global_props = d.get_global_properties();

    d.modify(global_props, [&](global_property_object& gpo)
    {
      gpo.parameters.starting_cycle_asset_amount = op.new_amount;
    });

    return {};
  } FC_CAPTURE_AND_RETHROW((op))
}

} } // graphene::chain
