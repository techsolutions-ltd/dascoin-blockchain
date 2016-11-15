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
#include <graphene/chain/transfer_evaluator.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/exceptions.hpp>
#include <graphene/chain/hardfork.hpp>
#include <graphene/chain/is_authorized_asset.hpp>

namespace graphene { namespace chain {
void_result transfer_evaluator::do_evaluate( const transfer_operation& op )
{ try {

   const database& d = db();

   const account_object& from_account    = op.from(d);
   const account_object& to_account      = op.to(d);
   const asset_object&   asset_type      = op.amount.asset_id(d);

   try {

      GRAPHENE_ASSERT(
         is_authorized_asset( d, from_account, asset_type ),
         transfer_from_account_not_whitelisted,
         "'from' account ${from} is not whitelisted for asset ${asset}",
         ("from",op.from)
         ("asset",op.amount.asset_id)
         );
      GRAPHENE_ASSERT(
         is_authorized_asset( d, to_account, asset_type ),
         transfer_to_account_not_whitelisted,
         "'to' account ${to} is not whitelisted for asset ${asset}",
         ("to",op.to)
         ("asset",op.amount.asset_id)
         );

      if( asset_type.is_transfer_restricted() )
      {
         GRAPHENE_ASSERT(
            from_account.id == asset_type.issuer || to_account.id == asset_type.issuer,
            transfer_restricted_transfer_asset,
            "Asset {asset} has transfer_restricted flag enabled",
            ("asset", op.amount.asset_id)
          );
      }

      bool insufficient_balance = d.get_balance( from_account, asset_type ).amount >= op.amount.amount;
      FC_ASSERT( insufficient_balance,
                 "Insufficient Balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
                 ("a",from_account.name)("t",to_account.name)("total_transfer",d.to_pretty_string(op.amount))("balance",d.to_pretty_string(d.get_balance(from_account, asset_type))) );

      return void_result();
   } FC_RETHROW_EXCEPTIONS( error, "Unable to transfer ${a} from ${f} to ${t}", ("a",d.to_pretty_string(op.amount))("f",op.from(d).name)("t",op.to(d).name) );

}  FC_CAPTURE_AND_RETHROW( (op) ) }

void_result transfer_evaluator::do_apply( const transfer_operation& o )
{ try {
   db().adjust_balance( o.from, -o.amount );
   db().adjust_balance( o.to, o.amount );
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result override_transfer_evaluator::do_evaluate( const override_transfer_operation& op )
{ try {
   const database& d = db();

   const asset_object&   asset_type      = op.amount.asset_id(d);
   GRAPHENE_ASSERT(
      asset_type.can_override(),
      override_transfer_not_permitted,
      "override_transfer not permitted for asset ${asset}",
      ("asset", op.amount.asset_id)
      );
   FC_ASSERT( asset_type.issuer == op.issuer );

   const account_object& from_account    = op.from(d);
   const account_object& to_account      = op.to(d);

   FC_ASSERT( is_authorized_asset( d, to_account, asset_type ) );
   FC_ASSERT( is_authorized_asset( d, from_account, asset_type ) );

   if( d.head_block_time() <= HARDFORK_419_TIME )
   {
      FC_ASSERT( is_authorized_asset( d, from_account, asset_type ) );
   }
   // the above becomes no-op after hardfork because this check will then be performed in evaluator

   FC_ASSERT( d.get_balance( from_account, asset_type ).amount >= op.amount.amount,
              "", ("total_transfer",op.amount)("balance",d.get_balance(from_account, asset_type).amount) );

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result override_transfer_evaluator::do_apply( const override_transfer_operation& o )
{ try {
   db().adjust_balance( o.from, -o.amount );
   db().adjust_balance( o.to, o.amount );
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result transfer_cycles_evaluator::do_evaluate(const transfer_cycles_operation& op)
{ try {
   const database& d = db();
   const account_object& from_account    = op.from_wallet(d);
   const account_object& to_account      = op.to_vault(d);
   share_type from_balance = d.get_cycle_balance(from_account);

   FC_ASSERT( from_account.is_wallet(), "Source '${f}'' must be a wallet account", ("f", from_account.name) );
   FC_ASSERT( to_account.is_vault(), "Destination '${t}'' must be a vault account ", ("t", to_account.name) );

   FC_ASSERT( from_balance >= op.amount,
              "Insufficient Balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_account.name)
              ("t",to_account.name)
              ("total_transfer",op.amount)
              ("balance",from_balance)
            );
   return void_result();

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_cycles_evaluator::do_apply(const transfer_cycles_operation& o)
{ try {
   database& d = db();

   d.adjust_cycle_balance( o.from_wallet, -o.amount );
   d.adjust_cycle_balance( o.to_vault, o.amount );
   return void_result();

} FC_CAPTURE_AND_RETHROW((o)) }

void_result transfer_vault_to_wallet_evaluator::do_evaluate(const transfer_vault_to_wallet_operation& op)
{ try {
   const database& d = db();

   const account_object& from_account = op.from_vault(d);
   const account_object& to_account = op.to_wallet(d);
   const asset_object& asset_type = op.asset_to_transfer.asset_id(d);

   // Must be VAULT --> WALLET:
   FC_ASSERT( from_account.is_vault(), "Source '${f}'' must be a vault account", ("f", from_account.name) );
   FC_ASSERT( to_account.is_wallet(), "Destination '${t}' must be a wallet account", ("t", to_account.name) );

   // Must have sufficient balance:
   asset from_balance = d.get_balance_and_check_limit(from_account, asset_type, op.asset_to_transfer.amount);
   bool insufficient_balance = from_balance.amount >= op.asset_to_transfer.amount;
   FC_ASSERT( insufficient_balance,
              "Insufficient balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_account.name)
              ("t",to_account.name)
              ("total_transfer",d.to_pretty_string(op.asset_to_transfer))
              ("balance",d.to_pretty_string(from_balance))
            );
   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_vault_to_wallet_evaluator::do_apply(const transfer_vault_to_wallet_operation& o)
{ try {
   db().adjust_balance(o.from_vault, -o.asset_to_transfer, /* modify_limit = */ true);
   db().adjust_balance(o.to_wallet, o.asset_to_transfer);
   return {};
} FC_CAPTURE_AND_RETHROW((o)) }

void_result transfer_wallet_to_vault_evaluator::do_evaluate(const transfer_wallet_to_vault_operation& op)
{ try {
   const database& d = db();

   const account_object& from_account = op.from_wallet(d);
   const account_object& to_account = op.to_vault(d);
   const asset_object& asset_type = op.asset_to_transfer.asset_id(d);

   // Must be WALLET --> VAULT:
   FC_ASSERT( from_account.is_wallet(), "Source '${f}'' must be a wallet account", ("f", from_account.name) );
   FC_ASSERT( to_account.is_vault(), "Destination '${t}' must be a vault account", ("t", to_account.name) );

   // Must have sufficient balance:
   asset from_balance = d.get_balance(from_account, asset_type);
   bool insufficient_balance = from_balance.amount >= op.asset_to_transfer.amount;
   FC_ASSERT( insufficient_balance,
              "Insufficient balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_account.name)
              ("t",to_account.name)
              ("total_transfer",d.to_pretty_string(op.asset_to_transfer))
              ("balance",d.to_pretty_string(from_balance))
            );
   return {};
} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_wallet_to_vault_evaluator::do_apply(const transfer_wallet_to_vault_operation& o)
{ try {
   db().adjust_balance(o.from_wallet, -o.asset_to_transfer);
   db().adjust_balance(o.to_vault, o.asset_to_transfer);
   return {};
} FC_CAPTURE_AND_RETHROW((o)) }

} } // graphene::chain
