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

      // Check if we are transferring dascoin
      FC_ASSERT( op.amount.asset_id == d.get_dascoin_asset_id(), "Can only transfer dascoins" );

     // Check if account types are valid
      FC_ASSERT( (from_account.is_wallet() && to_account.is_custodian()) ||
                 (from_account.is_custodian() && to_account.is_wallet()),
                  "One of the accounts must be a wallet account and other one must be a custodian account"  );

     // Check if there is enough cash balance in the source account
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

void_result transfer_vault_to_wallet_evaluator::do_evaluate(const transfer_vault_to_wallet_operation& op)
{ try {
   const database& d = db();

   // Check if we are transferring web assets or dascoin:
   // NOTE: this check must be modified to apply for every kind of web asset there is.
   FC_ASSERT ( op.asset_to_transfer.asset_id == d.get_web_asset_id() || op.asset_to_transfer.asset_id == d.get_dascoin_asset_id(),
               "Can only transfer web assets or dascoins" );

   // Check if the accounts exist:
   const account_object& from_acc_obj = op.from_vault(d);
   const account_object& to_acc_obj = op.to_wallet(d);

   // Must be VAULT --> WALLET:
   FC_ASSERT( from_acc_obj.is_vault(), "Source '${f}' must be a vault account", ("f", from_acc_obj.name) );
   FC_ASSERT( to_acc_obj.is_wallet(), "Destination '${t}' must be a wallet account", ("t", to_acc_obj.name) );

      // Accounts must be tethered:
   FC_ASSERT( from_acc_obj.has_in_parents(op.to_wallet),
              "Accounts '${f}' and '${t}' must be tethered",
              ("f", from_acc_obj.name)
              ("t", to_acc_obj.name)
            );

   // Get both balance objects:
   const auto& from_balance_obj = d.get_balance_object(op.from_vault, op.asset_to_transfer.asset_id);
   const auto& to_balance_obj = d.get_balance_object(op.to_wallet, op.asset_to_transfer.asset_id);

   if (op.asset_to_transfer.asset_id == d.get_dascoin_asset_id())
      FC_ASSERT( op.reserved_to_transfer == 0, "Cannot transfer reserved dascoin");

   // Check if we have enough cash balance in the FROM account:
   FC_ASSERT( from_balance_obj.balance >= op.asset_to_transfer.amount,
              "Insufficient balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_acc_obj.name)
              ("t",to_acc_obj.name)
              ("total_transfer",d.to_pretty_string(op.asset_to_transfer))
              ("balance",d.to_pretty_string(from_balance_obj.get_balance()))
            );

   // Check if we have enough reserved balance in the FROM account:
   FC_ASSERT( from_balance_obj.reserved >= op.reserved_to_transfer,
              "Insufficient reserved balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_acc_obj.name)
              ("t",to_acc_obj.name)
              ("total_transfer",d.to_pretty_string(asset(op.reserved_to_transfer, d.get_web_asset_id())))
              ("balance",d.to_pretty_string(from_balance_obj.get_reserved_balance()))
            );

   // If dascoin is being transferred, check daily limit constraint:
   if ( !from_acc_obj.disable_vault_to_wallet_limit && op.asset_to_transfer.asset_id == d.get_dascoin_asset_id() )
   {
      FC_ASSERT( from_balance_obj.spent + op.asset_to_transfer.amount <= from_balance_obj.limit,
                 "Cash limit has been exceeded, ${spent}/${max} on account ${a}",
                 ("a",from_acc_obj.name)
                 ("spent",d.to_pretty_string(from_balance_obj.get_spent_balance()))
                 ("max",d.to_pretty_string(asset(from_balance_obj.limit, op.asset_to_transfer.asset_id)))
               );
   }

   from_balance_obj_ = &from_balance_obj;
   to_balance_obj_ = &to_balance_obj;
   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_vault_to_wallet_evaluator::do_apply(const transfer_vault_to_wallet_operation& op)
{ try {
   auto& d = db();

   d.modify(*from_balance_obj_, [&](account_balance_object& from_b){
    from_b.balance -= op.asset_to_transfer.amount;
    from_b.reserved -= op.reserved_to_transfer;
    from_b.spent += (op.asset_to_transfer.amount + op.reserved_to_transfer);
   });

   d.modify(*to_balance_obj_, [&](account_balance_object& to_b){
    to_b.balance += op.asset_to_transfer.amount;
    to_b.reserved += op.reserved_to_transfer;
   });

   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_wallet_to_vault_evaluator::do_evaluate(const transfer_wallet_to_vault_operation& op)
{ try {
   const database& d = db();

   // Check if we are transferring web assets or dascoin:
   // NOTE: this check must be modified to apply for every kind of web asset there is.
   FC_ASSERT ( op.asset_to_transfer.asset_id == d.get_web_asset_id() || op.asset_to_transfer.asset_id == d.get_dascoin_asset_id(),
               "Can only transfer web assets or dascoins" );

   // Check if the accounts exist:
   const account_object& from_acc_obj = op.from_wallet(d);
   const account_object& to_acc_obj = op.to_vault(d);

   // Must be WALLET --> VAULT:
   FC_ASSERT( from_acc_obj.is_wallet(), "Source '${f}' must be a wallet account", ("f", from_acc_obj.name) );
   FC_ASSERT( to_acc_obj.is_vault(), "Destination '${t}' must be a vault account", ("t", to_acc_obj.name) );

   // Accounts must be tethered:
   FC_ASSERT( from_acc_obj.has_in_vault(op.to_vault), "Accounts '${f}' and '${t}' must be tethered",
              ("f", from_acc_obj.name)
              ("t", to_acc_obj.name)
            );

   // Get both balance objects:
   const auto& from_balance_obj = d.get_balance_object(op.from_wallet, op.asset_to_transfer.asset_id);
   const auto& to_balance_obj = d.get_balance_object(op.to_vault, op.asset_to_transfer.asset_id);

   if (op.asset_to_transfer.asset_id == d.get_dascoin_asset_id())
       FC_ASSERT( op.reserved_to_transfer == 0, "Cannot transfer reserved dascoin");

   // Check if we have enough balance in the FROM account:
   FC_ASSERT( from_balance_obj.balance >= op.asset_to_transfer.amount,
              "Insufficient balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_acc_obj.name)
              ("t",to_acc_obj.name)
              ("total_transfer",d.to_pretty_string(op.asset_to_transfer))
              ("balance",d.to_pretty_string(from_balance_obj.get_balance()))
            );

   // Check if we have enough reserved balance in the FROM account:
   FC_ASSERT( from_balance_obj.reserved >= op.reserved_to_transfer,
              "Insufficient reserved balance: ${balance}, unable to transfer '${total_transfer}' from account '${a}' to '${t}'",
              ("a",from_acc_obj.name)
              ("t",to_acc_obj.name)
              ("total_transfer",d.to_pretty_string(asset(op.reserved_to_transfer, d.get_web_asset_id())))
              ("balance",d.to_pretty_string(from_balance_obj.get_reserved_balance()))
            );

   from_balance_obj_ = &from_balance_obj;
   to_balance_obj_ = &to_balance_obj;
   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result transfer_wallet_to_vault_evaluator::do_apply(const transfer_wallet_to_vault_operation& op)
{ try {
   auto& d = db();

   d.modify(*from_balance_obj_, [&](account_balance_object& from_b){
    from_b.balance -= op.asset_to_transfer.amount;
    from_b.reserved -= op.reserved_to_transfer;
   });

   d.modify(*to_balance_obj_, [&](account_balance_object& to_b){
    to_b.balance += op.asset_to_transfer.amount;
    to_b.reserved += op.reserved_to_transfer;
   });

   return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_euro_limit_evaluator::do_evaluate(const operation_type &op)
{ try {
   const database& d = db();

   // from this moment there are no vault limits and this operation is deprecated
   if(d.head_block_time() >= HARDFORK_EXEX_102_TIME)
      FC_ASSERT( false, "This operation is deprecated!");

   const auto license_admin_id = d.get_global_properties().authorities.license_administrator;

   const account_object& authority_obj = op.authority(d);
   const account_object& acc_obj = op.account(d);

   d.perform_chain_authority_check("license administration", license_admin_id, authority_obj);

   FC_ASSERT( acc_obj.is_vault(), "Account '${acc_id}' needs to be a vault account", ("acc_id", acc_obj.id) );
   _account_obj = &acc_obj;

   return {};
} FC_CAPTURE_AND_RETHROW((op)) }

void_result update_euro_limit_evaluator::do_apply(const operation_type &op)
{ try {
   auto& d = db();

   d.modify(*_account_obj, [op](account_object &acc_obj){
      acc_obj.disable_vault_to_wallet_limit = op.disable_limit;
   });

   return {};
} FC_CAPTURE_AND_RETHROW((op)) }

void_result remove_vault_limit_evaluator::do_evaluate(const operation_type &op)
{ try {
   const database& d = db();

   const auto license_admin_id = d.get_global_properties().authorities.license_administrator;

   const account_object& authority_obj = op.authority(d);

   d.perform_chain_authority_check("license administration", license_admin_id, authority_obj);

   return {};
} FC_CAPTURE_AND_RETHROW((op)) }

void_result remove_vault_limit_evaluator::do_apply(const operation_type &op)
{ try {
   auto& d = db();

   d.remove_limit_from_all_vaults();

   return {};
} FC_CAPTURE_AND_RETHROW((op)) }

} } // graphene::chain
