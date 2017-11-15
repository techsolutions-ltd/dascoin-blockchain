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
#include <graphene/chain/witness_evaluator.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/protocol/vote.hpp>

namespace graphene { namespace chain {

void_result witness_create_evaluator::do_evaluate( const witness_create_operation& op )
{ try {
   FC_ASSERT(db().get(op.witness_account).is_lifetime_member());
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type witness_create_evaluator::do_apply( const witness_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::witness);
   });

   const auto& new_witness_object = db().create<witness_object>( [&]( witness_object& obj ){
         obj.witness_account  = op.witness_account;
         obj.signing_key      = op.block_signing_key;
         obj.vote_id          = vote_id;
         obj.url              = op.url;
   });
   return new_witness_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result witness_update_evaluator::do_evaluate( const witness_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.witness).witness_account == op.witness_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result witness_update_evaluator::do_apply( const witness_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.witness),
      [&]( witness_object& wit )
      {
         if( op.new_url.valid() )
            wit.url = *op.new_url;
         if( op.new_signing_key.valid() )
            wit.signing_key = *op.new_signing_key;
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void perform_root_authority_check(database& db, const account_id_type& authority_account_id)
{
   FC_ASSERT(db.get_dynamic_global_properties().is_root_authority_enabled_flag, "Your authority is deprecated!");

   const auto root_administrator_id = db.get_global_properties().authorities.root_administrator;
   const auto& op_authority_account_obj = authority_account_id(db);

   db.perform_chain_authority_check("root authority", root_administrator_id, op_authority_account_obj);
}

void_result remove_root_authority_evaluator::do_evaluate( const remove_root_authority_operation& op )
{ try {
   database& _db = db();
   perform_root_authority_check(_db, op.root_account);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result remove_root_authority_evaluator::do_apply( const remove_root_authority_operation& op )
{ try {
   database& _db = db();
   _db.modify(
         _db.get_dynamic_global_properties(),
         [&](dynamic_global_property_object& dgp)
         {
            dgp.is_root_authority_enabled_flag = false;
         });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result create_witness_evaluator::do_evaluate( const create_witness_operation& op )
{ try {

   database& _db = db();
   perform_root_authority_check(_db, op.authority);

   const account_object& account_obj= op.witness_account(_db);
   FC_ASSERT(account_obj.kind == account_kind::special, "Wrong kind of account!");

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type create_witness_evaluator::do_apply( const create_witness_operation& op )
{ try {

   const auto& new_witness_object = db().create<witness_object>( [&]( witness_object& obj ){
         obj.witness_account  = op.witness_account;
         obj.signing_key      = op.block_signing_key;
         obj.url              = op.url;
   });
   return new_witness_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result update_witness_evaluator::do_evaluate( const update_witness_operation& op )
{ try {
   database& _db = db();
   perform_root_authority_check(_db, op.authority);

   const auto& wit = op.witness(_db);
   if(op.witness_account.valid())
   {
      const auto& account_obj = (*op.witness_account)(_db);
      FC_ASSERT(account_obj.kind == account_kind::special, "Wrong kind of account!");
   }

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result update_witness_evaluator::do_apply( const update_witness_operation& op )
{ try {

   const auto& new_witness_delegate_data_object = db().create<witness_delegate_data_object>( [&]( witness_delegate_data_object& obj ){

      update_witness_delegate_data uwdd;
      uwdd.witness = op.witness;
      if(op.witness_account.valid())
         uwdd.witness_account    = *op.witness_account;
      if(op.block_signing_key.valid())
         uwdd.block_signing_key  = *op.block_signing_key;
      if(op.url.valid())
         uwdd.url                = *op.url;

      obj.data = uwdd;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result remove_witness_evaluator::do_evaluate( const remove_witness_operation& op )
{ try {
   database& _db = db();
   perform_root_authority_check(_db, op.authority);

   const auto& wit = op.witness(_db);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result remove_witness_evaluator::do_apply( const remove_witness_operation& op )
{ try {

   const auto& new_witness_delegate_data_object = db().create<witness_delegate_data_object>( [&]( witness_delegate_data_object& obj ){

      remove_witness_delegate_data rwdd;
      rwdd.witness = op.witness;

      obj.data = rwdd;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result activate_witness_evaluator::do_evaluate( const activate_witness_operation& op )
{ try {
   database& _db = db();
   perform_root_authority_check(_db, op.authority);
   const auto& wit = op.witness(_db);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result activate_witness_evaluator::do_apply( const activate_witness_operation& op )
{ try {

   const auto& new_witness_delegate_data_object = db().create<witness_delegate_data_object>( [&]( witness_delegate_data_object& obj ){

      activate_witness_delegate_data awdd;
      awdd.witness = op.witness;

      obj.data = awdd;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result deactivate_witness_evaluator::do_evaluate( const deactivate_witness_operation& op )
{ try {
   database& _db = db();
   perform_root_authority_check(_db, op.authority);
   const auto& wit = op.witness(_db);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result deactivate_witness_evaluator::do_apply( const deactivate_witness_operation& op )
{ try {
   const auto& new_witness_delegate_data_object = db().create<witness_delegate_data_object>( [&]( witness_delegate_data_object& obj ){

      deactivate_witness_delegate_data dwdd;
      dwdd.witness = op.witness;

      obj.data = dwdd;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void witness_delegate_data_visitor::operator ()(const update_witness_delegate_data& o)
{
   try {
      db.modify(
         db.get(o.witness), [&]( witness_object& obj ){
            if(o.witness_account.valid())
               obj.witness_account  = *o.witness_account;

            if(o.block_signing_key.valid())
               obj.signing_key      = *o.block_signing_key;

            if(o.url.valid())
               obj.url              = *o.url;
      });
} FC_CAPTURE_AND_RETHROW( (o) ) }

void witness_delegate_data_visitor::operator ()(const remove_witness_delegate_data& o)
{
   try {
      auto& obj = db.get(o.witness);
      db.remove(obj);
} FC_CAPTURE_AND_RETHROW( (o) ) }

void witness_delegate_data_visitor::operator ()(const activate_witness_delegate_data& o)
{
   try {
      db.modify(db.get_global_properties(), [&](global_property_object& gpo) {
         gpo.active_witnesses.insert(o.witness);
      });
} FC_CAPTURE_AND_RETHROW( (o) ) }

void witness_delegate_data_visitor::operator ()(const deactivate_witness_delegate_data& o)
{
   try {
      db.modify(db.get_global_properties(), [&](global_property_object& gpo) {
         auto itr = gpo.active_witnesses.find(o.witness);
         if(itr != gpo.active_witnesses.end())
            gpo.active_witnesses.erase(itr);
      });
} FC_CAPTURE_AND_RETHROW( (o) ) }


} } // graphene::chain
