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

#include <graphene/chain/asset_object.hpp>
#include <graphene/chain/chain_property_object.hpp>
#include <graphene/chain/global_property_object.hpp>

#include <fc/smart_ref_impl.hpp>

namespace graphene { namespace chain {

const asset_object& database::get_core_asset() const
{
   return get(asset_id_type());
}

const asset_object& database::get_web_asset() const
{
   return get(asset_id_type(DASCOIN_WEB_ASSET_INDEX));
}

asset_id_type database::get_web_asset_id() const
{
   return asset_id_type(DASCOIN_WEB_ASSET_INDEX);
}

const asset_object& database::get_dascoin_asset() const
{
   return get(asset_id_type(DASCOIN_DASCOIN_INDEX));
}

asset_id_type database::get_dascoin_asset_id() const
{
   return asset_id_type(DASCOIN_DASCOIN_INDEX);
}

const global_property_object& database::get_global_properties()const
{
   return get( global_property_id_type() );
}

const chain_property_object& database::get_chain_properties()const
{
   return get( chain_property_id_type() );
}

const chain_authorities& database::get_chain_authorities() const
{
   return get_global_properties().authorities;
}

const dynamic_global_property_object&database::get_dynamic_global_properties() const
{
   return get( dynamic_global_property_id_type() );
}

const fee_schedule&  database::current_fee_schedule()const
{
   return get_global_properties().parameters.current_fees;
}

time_point_sec database::head_block_time()const
{
   return get( dynamic_global_property_id_type() ).time;
}

uint32_t database::head_block_num()const
{
   return get( dynamic_global_property_id_type() ).head_block_number;
}

block_id_type database::head_block_id()const
{
   return get( dynamic_global_property_id_type() ).head_block_id;
}

decltype( chain_parameters::block_interval ) database::block_interval( )const
{
   return get_global_properties().parameters.block_interval;
}

const chain_id_type& database::get_chain_id( )const
{
   return get_chain_properties().chain_id;
}

const node_property_object& database::get_node_properties()const
{
   return _node_property_object;
}

node_property_object& database::node_properties()
{
   return _node_property_object;
}

uint32_t database::last_non_undoable_block_num() const
{
   ilog("DEBUG>>> head_block_num = ${hbn} _undo_db.size() = ${uds}", ("hbn",head_block_num())("uds",_undo_db.size()));
   return head_block_num() - _undo_db.size();
}

account_id_type database::get_account_id(const string& name)
{
   const auto& accounts_by_name = get_index_type<account_index>().indices().get<by_name>();
   auto itr = accounts_by_name.find(name);
   FC_ASSERT( itr != accounts_by_name.end(),
              "Unable to find account '${acct}'. Did you forget to add a record for it to initial_accounts?",
              ("acct", name)
            );
   return itr->get_id();
}

asset_id_type database::get_asset_id(const string& symbol)
{
   const auto& assets_by_symbol = get_index_type<asset_index>().indices().get<by_symbol>();
   auto itr = assets_by_symbol.find(symbol);

   // TODO: This is temporary for handling BTS snapshot
   if( symbol == "BTS" )
       itr = assets_by_symbol.find(GRAPHENE_SYMBOL);

   FC_ASSERT( itr != assets_by_symbol.end(),
              "Unable to find asset '${sym}'. Did you forget to add a record for it to initial_assets?",
              ("sym", symbol)
            );

   return itr->get_id();
}



} }
