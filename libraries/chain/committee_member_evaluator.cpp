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
#include <graphene/chain/committee_member_evaluator.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/account_object.hpp>
#include <graphene/chain/protocol/fee_schedule.hpp>
#include <graphene/chain/protocol/vote.hpp>
#include <graphene/chain/transaction_evaluation_state.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

void_result committee_member_create_evaluator::do_evaluate( const committee_member_create_operation& op )
{ try {
   FC_ASSERT(db().get(op.committee_member_account).is_lifetime_member());
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

object_id_type committee_member_create_evaluator::do_apply( const committee_member_create_operation& op )
{ try {
   vote_id_type vote_id;
   db().modify(db().get_global_properties(), [&vote_id](global_property_object& p) {
      vote_id = get_next_vote_id(p, vote_id_type::committee);
   });

   const auto& new_del_object = db().create<committee_member_object>( [&]( committee_member_object& obj ){
         obj.committee_member_account   = op.committee_member_account;
         obj.vote_id            = vote_id;
         obj.url                = op.url;
   });
   return new_del_object.id;
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_evaluator::do_evaluate( const committee_member_update_operation& op )
{ try {
   FC_ASSERT(db().get(op.committee_member).committee_member_account == op.committee_member_account);
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_evaluator::do_apply( const committee_member_update_operation& op )
{ try {
   database& _db = db();
   _db.modify(
      _db.get(op.committee_member),
      [&]( committee_member_object& com )
      {
         if( op.new_url.valid() )
            com.url = *op.new_url;
      });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (op) ) }

void_result committee_member_update_global_parameters_evaluator::do_evaluate(const committee_member_update_global_parameters_operation& o)
{ try {
   FC_ASSERT(trx_state->_is_proposed_trx);

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_global_parameters_evaluator::do_apply(const committee_member_update_global_parameters_operation& o)
{ try {
   db().modify(db().get_global_properties(), [&o](global_property_object& p) {
      p.pending_parameters = o.new_parameters;
   });

   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) ) }

void_result committee_member_update_license_issuer_evaluator::do_evaluate(const committee_member_update_license_issuer_operation& o)
{ try {
   // Check if the license issuer is a lifetime member.
   FC_ASSERT( db().get(o.license_issuer).is_lifetime_member() );
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) )}

void_result committee_member_update_license_issuer_evaluator::do_apply(const committee_member_update_license_issuer_operation& o)
{ try {
   // Modifiy the global properties object and set the new license issuing authority.
   db().modify(db().get_global_properties(), [&o](global_property_object& p) {
      p.authorities.license_issuer = o.license_issuer;
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) )}

void_result committee_member_update_license_authenticator_evaluator::do_evaluate(const committee_member_update_license_authenticator_operation& o)
{ try {
   // Check if the license authenticator is a lifetime member.
   FC_ASSERT( db().get(o.license_authenticator).is_lifetime_member() );
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) )}

void_result committee_member_update_license_authenticator_evaluator::do_apply(const committee_member_update_license_authenticator_operation& o)
{ try {
   // Modifiy the global properties object and set the new license authentication authority.
   db().modify(db().get_global_properties(), [&o](global_property_object& p) {
      p.authorities.license_authenticator = o.license_authenticator;
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) )}

void_result committee_member_update_account_registrar_evaluator::do_evaluate(const committee_member_update_account_registrar_operation& o)
{ try {
   // Check if the license authenticator is a lifetime member.
   FC_ASSERT( db().get(o.registrar).is_lifetime_member() );
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) )}

void_result committee_member_update_account_registrar_evaluator::do_apply(const committee_member_update_account_registrar_operation& o)
{ try {
   // Modifiy the global properties object and set the new license authentication authority.
   db().modify(db().get_global_properties(), [&o](global_property_object& p) {
      p.authorities.registrar = o.registrar;
   });
   return void_result();
} FC_CAPTURE_AND_RETHROW( (o) )}

} } // graphene::chain
