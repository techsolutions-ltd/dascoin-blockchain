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
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

object_id_type committee_member_create_evaluator::do_apply( const committee_member_create_operation& op )
{ try {
  auto& d = db();
  vote_id_type vote_id;

  d.modify(d.get_global_properties(), [&vote_id](global_property_object& p) {
    vote_id = get_next_vote_id(p, vote_id_type::committee);
  });

  return d.create<committee_member_object>( [&]( committee_member_object& obj ){
       obj.committee_member_account   = op.committee_member_account;
       obj.vote_id            = vote_id;
       obj.url                = op.url;
  }).id;

} FC_CAPTURE_AND_RETHROW((op)) }

void_result committee_member_update_evaluator::do_evaluate( const committee_member_update_operation& op )
{ try {

  FC_ASSERT(db().get(op.committee_member).committee_member_account == op.committee_member_account);
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result committee_member_update_evaluator::do_apply( const committee_member_update_operation& op )
{ try {
  database& d = db();

  d.modify(op.committee_member(d), [&](committee_member_object& cmo) {
    if( op.new_url.valid() )
      cmo.url = *op.new_url;
  });
  return {};

} FC_CAPTURE_AND_RETHROW((op)) }

void_result committee_member_update_global_parameters_evaluator::do_evaluate(const committee_member_update_global_parameters_operation& o)
{ try {

  FC_ASSERT(trx_state->_is_proposed_trx);
  return {};

} FC_CAPTURE_AND_RETHROW((o)) }

void_result committee_member_update_global_parameters_evaluator::do_apply(const committee_member_update_global_parameters_operation& o)
{ try {
  auto& d = db();

  d.modify(d.get_global_properties(), [&o](global_property_object& gpo) {
    gpo.pending_parameters = o.new_parameters;
  });

  return {};
} FC_CAPTURE_AND_RETHROW((o)) }

void_result board_update_chain_authority_evaluator::do_evaluate(const board_update_chain_authority_operation& op)
{ try {
  const auto& d = db();
  const auto& account_obj = op.account(d);

  FC_ASSERT( account_obj.is_lifetime_member(), "Account ${n} must be a lifetime member.", ("n", account_obj.name));

  return {};

} FC_CAPTURE_AND_RETHROW((op))}

void_result board_update_chain_authority_evaluator::do_apply(const board_update_chain_authority_operation& op)
{ try {
   using namespace graphene::chain::util;
   auto& d = db();

   auto kind = convert_enum<chain_authority_kind>::from_fc_string(op.kind);
   d.modify(d.get_global_properties(), [&](global_property_object& gpo){
      gpo.authorities.set(kind, op.account);
   });

   return {};

} FC_CAPTURE_AND_RETHROW((op))}

} } // graphene::chain
